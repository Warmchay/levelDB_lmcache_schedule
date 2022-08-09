# LastLevel Cache Schedule

# 1 整体架构

<img src="https://raw.githubusercontent.com/Warmchay/img/main/%E6%88%AA%E5%B1%8F2022-08-09%20%E4%B8%8B%E5%8D%883.59.37.png" style="zoom:50%;" />

动机：最大层的SST的元数据占缓存空间最大，但是对读放大没有明显的收益

想法：将元数据缓存预算分割部分给最大层的数据缓存，整体减轻读放大的影响

实现：

- [x] 数据缓存的数据一致性保障：通过额外的线程比较 Memtable 中的数据

- [x] 对负载的自适应性，统计数据/元数据缓存的命中率，静态调整两个缓存的大小
- [x] 比较两个缓存缺失开销，构建 cost-benefit model 动态调整缓存大小
- [ ] 细粒度地调整 cost-benefit model

尝试：

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build .

./db_bench --benchmarks=fillrandom,stats,ycsba,stats --num=100000000 --write_buffer_size=33554432 --value_size=232 --histogram=1 --bloom_bits=10 --db=/mnt/ssd1/data_dir/
```

性能：( ycsba: 5000,000, ws_size: 5,000 )

```bash
back time: 5, resize time: 7 
init cost: 276453.000000, avg cost: 226532.714286
```



# 2 中间实验

> yscbc: 1000,000

| 划分比例 (最大层元数据缓存 : 数据缓存) | 数据缓存命中时间(ms) | 最大层元数据缓存命中时间(ms) | 最大层元数据缓存不命中时间(ms) | 命中比例(元数据缓存 ：数据缓存) 工作集大小为 2000 | 最大层数据命中时间求和 |
| :------------------------------------: | :------------------: | :--------------------------: | :----------------------------: | :-----------------------------------------------: | :--------------------: |
|               **1 : 0**                |          0           |           8488.610           |            9342.276            |                     2000 : 0                      |      **8488.610**      |
|                 2 : 1                  |       1052.840       |           7831.426           |            8745.962            |                     1957 : 43                     |        8884.266        |
|                 1 : 2                  |       1062.459       |           8906.995           |            9685.160            |                     1964 : 36                     |        9969.454        |
|                 1 : 1                  |       1051.075       |           8234.143           |            9158.801            |                    1891 : 109                     |        9285.218        |
|                 5 : 7                  |       1032.014       |          14659.862           |           15515.469            |                     1962 : 38                     |       15691.875        |
|                 99 : 1                 |       1047.138       |           7957.219           |            8830.800            |                     1917 : 83                     |        9004.357        |
|               **1 : 99**               |       1249.302       |           4029.348           |            5596.313            |                     1967 : 33                     |      **5278.650**      |
|              **1 : 999**               |       1248.413       |           4836.042           |            6409.828            |                     1948 : 52                     |      **6084.455**      |

> 下述的最大层元数据缓存指: 元数据缓存条目数 *  ( IB Size + FB Size)

结论：

1. 最大层数据缓存分得的最大层元数据缓存越多，如数据缓存分得 99/100 的初始元数据缓存时，**最大层读放大收益**降低了约 3000 ms
2. 分配比例位于 [1 :99 ~ 999] 间会有一个最优读放大收益值



# 3 建模准备

参考论文：

1. https://www.usenix.org/system/files/atc20-wu-fenggang.pdf
2. https://www.usenix.org/legacy/events/fast03/tech/full_papers/megiddo/megiddo.pdf

Dynamic Boundary:

![](https://raw.githubusercontent.com/Warmchay/img/main/%E6%88%AA%E5%B1%8F2022-08-09%20%E4%B8%8B%E5%8D%884.35.41.png)

## 3.1 整体时间

- 每个 I/O 访问 data cache (dc) 的时间 ( $T_{DC}$ ):
  
  $$
  data \ cache \ single \ IO\ (T_{DC})= \frac{dc\ miss\ time + dc \ hit\ time}{Accesses}
  $$

- 每个 I/O 访问 meta cache 的时间 ( $T_{LM}$ )：
  
  $$
  lastlevel\ metacache \ single\ IO\ (T_{LM}) = \frac{lastlevel\ miss \ time + lastlevel\ hit \ time + \{otherlevels\ hit \ time\}}{ Accesses}
  $$

  >  {otherlevels hit time}: 最大层除外的其他层 meta cache 命中时间总和

## 3.2 命中时间

如果命中 data cache 或 lastlevel meta cache 的单个 I/O 时间为:

- 访问 data cache 并命中的单个 I/O 耗时 $H_{DC}$:

$$
  H_{DC}=\frac{dc\ hit \ time}{dc \ hit\ Accesses}
$$

- 访问 lastlevel meta cache 并命中的单个 I/O 耗时 $H_{LM}$:

$$
  H_{LM}= \frac{dc\ miss + lastlevel\ hit\ time + \{otherlevels \ hit \ time\}  + data\ block\ read}{lastlevel\ hit\ Accesses}
$$
  

- 访问 lastlevel meta cache 并命中的单个 I/O 耗时 $\overline{H_{LM}}$:

$$
  \overline{H_{LM}} = \frac{dc\ miss +lastlevel\ miss\ time + \{otherlevels \ hit \ time\} + data\ block\ read}{lastlevel\ miss\ Accesses}
$$
  

因为最大层元数据缓存容量一定，分配策略的目的为：

1. 缓存尽量多命中
2. 单个 I/O 访问 levelDB 的读开销应减少



# 4 建模细化

## 4.1 代价最小

在固定的工作集 (working set) 中，代价和由三部分组成：数据缓存命中 + 最大层元数据缓存命中 + 最大层元数据缓存不命中

$$
cost = H_{DC}*Hit_{DC} + H_{LM} * Hit_{LM} + \overline{H_{LM} }*\overline{Hit_{LM} }
$$

其中 $\overline{Hit_{LM} } = ws\_size-Hit_{DC}-Hit_{LM}$

当没有分配 data cache 时，代价的最小值设定为 $cost_{init}$：

$$
cost_{init} = H_{LM} * Hit_{LM} + \overline{H_{LM} }*\overline{Hit_{LM} }
$$

代价最小的决定因素有两个：

1. 缓存要尽量命中，即 $\overline{Hit_{LM} }$  尽量少
2. 缓存命中的时间要少

代价最小的特殊情况是：

1. 如果新调整的 $cost\ge cost_{init}$时，调整 DC Cache 容量为 0 

## 4.2 缓存划分方向

设 p 为数据缓存和最大层元数据缓存平均命中时间的比值，即

$$
p = \frac{(H_{LM}-H_{DC})*Hit_{DC}}{(\overline{H_{LM}}-H_{LM})*\overline{Hit_{LM}}}
$$

由于data cache 的出现，会增加 lastlevel meta cache 缺失的风险，所以比较时应该将最大层元数据缓存缺失的原因加进来。

由 p 值处理三种情况：

1. p > 1, 命中 lastlevel meta cache 的开销更小，增大 lastlevel meta cache，减小 data cache
2. p < 1, 命中 lastlevel meta cache 的开销更大，增加 data cache，减小 lastlevel meta cache
3. p = 1, 不变

目的是为了提高缓存命中率，并减少命中开销

## 4.3 划分数值

缓存增加/减小的值还与工作集中缓存命中的相对比例 $\Delta$ 有关，缓存命中次数越多，说明其缓存的数据更热，被读的可能更大：

$$
\Delta = \frac{data\ cache\ hit + lastlevel\ meta\ cache\ hit}{2 * lastlevel\ meta\ cache\ hit}
$$

根据 $\Delta $ 和 p 值，可以得到四种情况，$R_{DC}$ 表示当前 data cache 的容量，$R_{LM}$ 表示当前 lastlevel meta cache 的容量：

1. $\Delta >= 1, p >1$，说明 data cache 命中多但开销大，减小的容量分给 lastlevel metacache

$$
   R_{LM} = R_{LM} + \Delta * \frac{R_{LM}}{R_{LM}+R_{DC}} \qquad \\
   R_{DC} = R_{DC} - \Delta * \frac{R_{LM}}{R_{LM}+R_{DC}} \\
$$

2. $\Delta >= 1, p < 1$，说明 data cache 命中多且开销小，增大 data cache 容量，减小 lastlevel metacache

$$
   R_{LM} = R_{LM} - \Delta * \frac{R_{DC}}{R_{LM}+R_{DC}} \qquad \\
   R_{DC} = R_{DC} + \Delta * \frac{R_{DC}}{R_{LM}+R_{DC}} \\
$$

3. $\Delta < 1, p > 1$，说明 data cache 命中不多且开销大，可以适当减小 data cache 容量

$$
   R_{LM} = R_{LM} + \frac{1}{\Delta} * \frac{R_{LM}}{R_{LM}+R_{DC}} \qquad \\ 
   R_{DC} = R_{DC} - \frac{1}{\Delta} * \frac{R_{LM}}{R_{LM}+R_{DC}}
$$

4. $\Delta < 1, p < 1$，说明 data cache 命中不多但开销小，需要增大 data cache 容量

$$
   R_{LM} = R_{LM} - \frac{1}{\Delta} * \frac{R_{DC}}{R_{LM}+R_{DC}} \qquad \\ 
   R_{DC} = R_{DC} + \frac{1}{\Delta}  * \frac{R_{DC}}{R_{LM}+R_{DC}}
$$
   

