// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Thread-safe (provides internal synchronization)

#ifndef STORAGE_LEVELDB_DB_TABLE_CACHE_H_
#define STORAGE_LEVELDB_DB_TABLE_CACHE_H_

#include <cstddef>
#include <cstdint>
#include <string>

#include "db/db_impl.h"
#include "db/dbformat.h"
#include "leveldb/cache.h"
#include "leveldb/table.h"
#include "port/port.h"

namespace leveldb {

class Env;

class TableCache {
 public:
  TableCache(const std::string& dbname, const Options& options, int entries);
  ~TableCache();

  // Return an iterator for the specified file number (the corresponding
  // file length must be exactly "file_size" bytes).  If "tableptr" is
  // non-null, also sets "*tableptr" to point to the Table object
  // underlying the returned iterator, or to nullptr if no Table object
  // underlies the returned iterator.  The returned "*tableptr" object is owned
  // by the cache and should not be deleted, and is valid for as long as the
  // returned iterator is live.
  Iterator* NewIterator(const ReadOptions& options, uint64_t file_number,
                        uint64_t file_size, size_t level, Table** tableptr = nullptr);  //22-7-11 增加层信息

  // If a seek to internal key "k" in specified file finds an entry,
  // call (*handle_result)(arg, found_key, found_value).
  Status Get(const ReadOptions& options, Env* env_, ReadStats* readstats, int level, uint64_t file_number,
             uint64_t file_size, const Slice& k, bool* fp_flag, bool& meta_hit, void* arg,
             void (*handle_result)(void*, const Slice&, const Slice&));  //22-7-11 增加一个参数标记元数据命中情况

  // Evict any entry for the specified file number
  void Evict(uint64_t file_number);

  size_t GetLastLevelSize() {
    return lastlevel_cache_->TotalCharge();
  }

  // Change-3 修改 lastlevel_cache 的大小
  void ResizeLastlevelCache(int capacity) {
    lastlevel_cache_->Resize(capacity);
  }

 private:
  Status FindTable(uint64_t file_number, uint64_t file_size, int level, bool tag, Cache::Handle**, bool& meta_hit);  // 22-7-11 增加一个记录元数据缓存是否命中的flag

  Env* const env_;
  const std::string dbname_;
  const Options& options_;
  Cache* cache_;
  Cache* lastlevel_cache_;  //22-7-11 将最大层元数据缓存与其他层剥离，便于控制其他层全命中和缓存空间分配
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_TABLE_CACHE_H_
