#pragma once

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <unistd.h>
#include <unordered_map>
#include <vector>

constexpr size_t BLOCK_SIZE = 4096 * 32; // Размер блока

struct CacheBlock {
  int fd;                 // Файловый дескриптор
  off_t block_index;      // Индекс блока в файле
  std::vector<char> data; // Данные блока
  bool dirty;             // Флаг модификации

  CacheBlock(int fd, off_t block_index, const std::vector<char> &data,
             bool dirty)
      : fd(fd), block_index(block_index), data(data), dirty(dirty) {}
};

class BlockCache {
private:
  size_t capacity;
  std::list<CacheBlock> cache_list;
  std::unordered_map<int,std::unordered_map<off_t, std::list<CacheBlock>::iterator>> cache_map;

  void evict();

  CacheBlock *getBlock(int fd, off_t block_index);

public:
  
  explicit BlockCache(size_t capacity) : capacity(capacity) {}

  std::vector<char> read(int fd, off_t block_index);
  void write(int fd, off_t block_index, const std::vector<char> &data);
  void fsync(int fd);
  void close(int fd);

};