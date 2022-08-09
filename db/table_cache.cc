// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/table_cache.h"
#include <db/db_impl.h>
#include <cstddef>
#include <cstdint>

#include "db/filename.h"
#include "leveldb/env.h"
#include "leveldb/table.h"
#include "util/coding.h"

namespace leveldb {

uint64_t num_meta = 0;

struct TableAndFile {
  RandomAccessFile* file;
  Table* table;
};

static void DeleteEntry(const Slice& key, void* value) {
  TableAndFile* tf = reinterpret_cast<TableAndFile*>(value);
  delete tf->table;
  delete tf->file;
  delete tf;
}

static void UnrefEntry(void* arg1, void* arg2) {
  Cache* cache = reinterpret_cast<Cache*>(arg1);
  Cache::Handle* h = reinterpret_cast<Cache::Handle*>(arg2);
  cache->Release(h);
}

TableCache::TableCache(const std::string& dbname, const Options& options,
                       int entries)
    : env_(options.env),
      dbname_(dbname),
      options_(options),
      cache_(NewLRUCache(entries)),
      lastlevel_cache_(NewLRUCache(entries*5)) {}

TableCache::~TableCache() { 
  delete cache_; 
  delete lastlevel_cache_;  //22-7-11
}

Status TableCache::FindTable(uint64_t file_number, uint64_t file_size, int level, bool tag,
                             Cache::Handle** handle, bool& meta_hit) {  //22-7-11 增加参数，获得元数据缓存命中情况
  Status s;
  char buf[sizeof(file_number)];
  EncodeFixed64(buf, file_number);
  Slice key(buf, sizeof(buf));
  // 22-3 统计元数据缓存hit/miss的开销
  uint64_t start_time = env_->NowMicros();
  if (level < config::kNumLevels - 1) {  // 22-7-11 改成2个cache
    *handle = cache_->Lookup(key);
  } else {
    *handle = lastlevel_cache_->Lookup(key);
  }
  
  uint64_t cache_lookup = env_->NowMicros();
  uint64_t meta_load, cache_insert, open_file;
  bool cache_miss = true;
  if (*handle == nullptr) {
    std::string fname = TableFileName(dbname_, file_number);
    RandomAccessFile* file = nullptr;
    Table* table = nullptr;
    s = env_->NewRandomAccessFile(fname, &file);
    if (!s.ok()) {
      std::string old_fname = SSTTableFileName(dbname_, file_number);
      if (env_->NewRandomAccessFile(old_fname, &file).ok()) {
        s = Status::OK();
      }
    }
    open_file = env_->NowMicros();
    if (s.ok()) {
      s = Table::Open(options_, file, file_size, &table);
    }
    if (tag) {
      meta_load = env_->NowMicros(); //22-3 累加cache miss的时间
      read_stats[level]._file_open += open_file - cache_lookup;
      read_stats[level]._metaload_micros += meta_load - open_file;
      read_stats[level]._Cache_miss += 1;
      // 22-6-17 统计总的索引数据读取量，在等大小KV下，BF大小应该相近
      read_stats[level].reads_IB += table->GetMetaSize();
    }

    if (!s.ok()) {
      assert(table == nullptr);
      delete file;
      // We do not cache error results so that if the error is transient,
      // or somebody repairs the file, we recover automatically.
    } else {
      TableAndFile* tf = new TableAndFile;
      tf->file = file;
      tf->table = table;
      if (level < config::kNumLevels - 1) {  // 22-7-11 改成2个cache
        *handle = cache_->Insert(key, tf, 1, &DeleteEntry);
      } else {
        *handle = lastlevel_cache_->Insert(key, tf, 1, &DeleteEntry);
      }
      cache_insert = env_->NowMicros();
      if (tag) {      
        read_stats[level]._cacheinsert_micros += cache_insert - meta_load; //22-3 累加cache insert的时间    
        read_stats[level].metaload_lastlevel_micros += cache_insert - start_time; // change-3 累加最大层元数据缓存未命中时间
      }
    }
    
  } else {
    // cache hit
    if (level == config::kNumLevels - 1) {
      meta_hit = true;  // 22-7-11 最大层命中
      // read_stats[level].metahit_lastlevel_micros += cache_lookup - start_time;
    }
    if (tag) {
      read_stats[level]._metahit_micros += cache_lookup - start_time;  //22-3 累加cache hit的查找时间
      read_stats[level]._Cache_hits += 1;
      cache_miss = false;
    }
  }
  if (tag && cache_miss) {
    num_meta++;
    // Q num_meta 表示的是什么，为什么 <= 20 的时候才打印呢？
    if (num_meta <= 20) {
      std::printf("level: %d, c_lookup: %lu, open_file: %lu,  read_meta: %lu, c_insert: %lu \n", 
        level, cache_lookup - start_time, open_file - cache_lookup, meta_load - open_file, cache_insert - meta_load);
    }
  }
  return s;
}

Iterator* TableCache::NewIterator(const ReadOptions& options,
                                  uint64_t file_number, uint64_t file_size, size_t level,
                                  Table** tableptr) {  //22-7-11 因为将最大层的元数据缓存解耦，因此需要level信息
  if (tableptr != nullptr) {
    *tableptr = nullptr;
  }

  Cache::Handle* handle = nullptr;
  bool meta_hit = false;
  Status s = FindTable(file_number, file_size, level, false, &handle, meta_hit);  //22-7-11
  if (!s.ok()) {
    return NewErrorIterator(s);
  }

  // 22-7-11 不同层访问不同cache
  bool lastlevel_flag = level == (config::kNumLevels - 1);
  Table* table = nullptr;
  if (lastlevel_flag) {
    table = reinterpret_cast<TableAndFile*>(lastlevel_cache_->Value(handle))->table;
  } else {
    table = reinterpret_cast<TableAndFile*>(cache_->Value(handle))->table;
  }

  Iterator* result = table->NewIterator(options);
  if (lastlevel_flag) {  // 22-7-11 缓存中的记录每lookup一次就要registercheanup一次
    result->RegisterCleanup(&UnrefEntry, lastlevel_cache_, handle);
  } else {
    result->RegisterCleanup(&UnrefEntry, cache_, handle);
  }
  
  if (tableptr != nullptr) {
    *tableptr = table;
  }
  return result;
}

Status TableCache::Get(const ReadOptions& options, Env* env_, ReadStats* readstats, int level, uint64_t file_number,
                       uint64_t file_size, const Slice& k, bool* fp_flag, bool& meta_hit, void* arg,
                       void (*handle_result)(void*, const Slice&,
                                             const Slice&)) {
  // 22-7-11 添加从数据缓存读取数据的逻辑
  Status s;
  Cache::Handle* handle = nullptr;
  bool lastlevel_flag = false;
  if (level == config::kNumLevels - 1) {
    // 最大层访问
    lastlevel_flag = true;
  }
  uint64_t start_time = env_->NowMicros();
  s = FindTable(file_number, file_size, level, true, &handle, meta_hit);  // 22-7-11 增加了元数据缓存是否命中的flag
  if (s.ok()) {
    Table* t = nullptr;  // 22-7-11 拆分最大层table cache
    if (lastlevel_flag) {
      t = reinterpret_cast<TableAndFile*>(lastlevel_cache_->Value(handle))->table;
    } else {
      t = reinterpret_cast<TableAndFile*>(cache_->Value(handle))->table;
    }

    s = t->InternalGet(options, env_, readstats, level, k, fp_flag, arg, handle_result);  // 22-4
      //22-7-11 缓存中的entry每lookup一次就要release一次
      if (lastlevel_flag) {
        lastlevel_cache_->Release(handle);     
      } else {
        cache_->Release(handle);
      }
    
  }
  if (lastlevel_flag) {
    if (meta_hit) {
      read_stats[level].metahit_lastlevel_micros += env_->NowMicros() - start_time;
    } else {
      read_stats[level].metaload_lastlevel_micros += env_->NowMicros() - start_time;
    }
  }

  return s;
}

void TableCache::Evict(uint64_t file_number) {
  char buf[sizeof(file_number)];
  EncodeFixed64(buf, file_number);
  cache_->Erase(Slice(buf, sizeof(buf)));
}

}  // namespace leveldb
