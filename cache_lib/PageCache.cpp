#include "PageCache.hpp"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <unistd.h>
#include <unordered_map>
#include <vector>

void BlockCache::evict() {
  if (cache_list.empty())
    return;

  CacheBlock &block = cache_list.front();
  if (block.dirty) {
    off_t offset = block.block_index * BLOCK_SIZE;
    if (pwrite(block.fd, block.data.data(), BLOCK_SIZE, offset) == -1) {
      perror("Eviction write failed");
    }
  }

  cache_map[block.fd].erase(block.block_index);
  if (cache_map[block.fd].empty()) {
    cache_map.erase(block.fd);
  }

  cache_list.pop_front();
}

CacheBlock *BlockCache::getBlock(int fd, off_t block_index) {
  if (this->cache_map.count(fd) && this->cache_map[fd].count(block_index)) {
    auto it = this->cache_map[fd][block_index];
    this->cache_list.splice(this->cache_list.end(), this->cache_list, it);
    return &(*it);
  }
  return nullptr;
}

std::vector<char> BlockCache::read(int fd, off_t block_index) {
  CacheBlock *block = getBlock(fd, block_index);
  if (block) {
    return block->data;
  }

  std::vector<char> buffer(BLOCK_SIZE);
  off_t offset = block_index * BLOCK_SIZE;
  ssize_t bytes_read = pread(fd, buffer.data(), BLOCK_SIZE, offset);
  if (bytes_read == -1) {
    perror("Read failed");
    return {};
  }
  buffer.resize(bytes_read);

  if (cache_list.size() >= capacity) {
    evict();
  }

  cache_list.emplace_back(fd, block_index, buffer, false);
  cache_map[fd][block_index] = std::prev(cache_list.end());

  return buffer;
}

void BlockCache::write(int fd, off_t block_index,
                       const std::vector<char> &data) {
  CacheBlock *block = getBlock(fd, block_index);
  if (block) {
    block->data = data;
    block->dirty = true;
    return;
  }

  if (cache_list.size() >= capacity) {
    evict();
  }

  cache_list.emplace_back(fd, block_index, data, true);
  cache_map[fd][block_index] = std::prev(cache_list.end());
}

void BlockCache::fsync(int fd) {
  if (!cache_map.count(fd))
    return;

  for (auto &[block_index, it] : cache_map[fd]) {
    CacheBlock &block = *it;
    if (block.dirty) {
      off_t offset = block.block_index * BLOCK_SIZE;
      if (pwrite(fd, block.data.data(), BLOCK_SIZE, offset) == -1) {
        perror("fsync write failed");
      }
      block.dirty = false;
    }
  }
}

void BlockCache::close(int fd) {
  if (!cache_map.count(fd))
    return;

  for (auto &[block_index, it] : cache_map[fd]) {
    CacheBlock &block = *it;
    if (block.dirty) {
      off_t offset = block.block_index * BLOCK_SIZE;
      if (pwrite(fd, block.data.data(), BLOCK_SIZE, offset) == -1) {
        perror("Close write failed");
      }
    }
  }

  cache_map.erase(fd);
  cache_list.remove_if(
      [fd](const CacheBlock &block) { return block.fd == fd; });
}
