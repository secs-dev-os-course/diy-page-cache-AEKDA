#include "PageCacheFS.hpp"
#include "PageCache.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <unordered_map>

// Обёртки для API
BlockCache cache(4096);

int lab2_open(const char *path) {
  int fd = open(path, O_RDWR | O_DIRECT);
  if (fd == -1) {
    perror("Open failed");
  }
  return fd;
}

int lab2_close(int fd) {
  cache.close(fd);
  return close(fd);
}

ssize_t lab2_read(int fd, void *buf, size_t count) {
  char *buffer = static_cast<char *>(buf);
  size_t bytes_read = 0;

  // Выравниваем временный буфер для работы с O_DIRECT
  void *aligned_buffer = nullptr;
  if (posix_memalign(&aligned_buffer, BLOCK_SIZE, BLOCK_SIZE) != 0) {
    perror("Failed to allocate aligned memory");
    return -1;
  }

  while (count > 0) {
    off_t block_index = lseek(fd, 0, SEEK_CUR) / BLOCK_SIZE;
    off_t block_offset = lseek(fd, 0, SEEK_CUR) % BLOCK_SIZE;
    size_t to_read = std::min(count, BLOCK_SIZE - block_offset);

    ssize_t result =
        pread(fd, aligned_buffer, BLOCK_SIZE, block_index * BLOCK_SIZE);
    if (result <= 0) {
      if (result < 0)
        perror("Read failed");
      break;
    }

    // Копируем нужную часть блока в пользовательский буфер
    std::memcpy(buffer + bytes_read,
                static_cast<char *>(aligned_buffer) + block_offset, to_read);
    bytes_read += to_read;
    count -= to_read;
    lseek(fd, to_read, SEEK_CUR);
  }

  free(aligned_buffer);
  return bytes_read;
}

ssize_t lab2_write(int fd, const void *buf, size_t count) {
  const char *buffer = static_cast<const char *>(buf);
  size_t bytes_written = 0;

  // Выравниваем временный буфер для работы с O_DIRECT
  void *aligned_buffer = nullptr;
  if (posix_memalign(&aligned_buffer, BLOCK_SIZE, BLOCK_SIZE) != 0) {
    perror("Failed to allocate aligned memory");
    return -1;
  }

  while (count > 0) {
    off_t block_index = lseek(fd, 0, SEEK_CUR) / BLOCK_SIZE;
    off_t block_offset = lseek(fd, 0, SEEK_CUR) % BLOCK_SIZE;
    size_t to_write = std::min(count, BLOCK_SIZE - block_offset);

    std::vector<char> block_data = cache.read(fd, block_index);
    if (block_data.empty()) {
      block_data.resize(BLOCK_SIZE, 0); // Если блока нет, создаём его с нулями
    }

    // Копируем данные из пользовательского буфера во временный буфер
    std::memcpy(static_cast<char *>(aligned_buffer), block_data.data(),
                BLOCK_SIZE);
    std::memcpy(static_cast<char *>(aligned_buffer) + block_offset,
                buffer + bytes_written, to_write);

    // Пишем обновлённый блок обратно
    ssize_t result =
        pwrite(fd, aligned_buffer, BLOCK_SIZE, block_index * BLOCK_SIZE);
    if (result <= 0) {
      if (result < 0)
        perror("Write failed");
      break;
    }

    cache.write(
        fd, block_index,
        std::vector<char>(static_cast<char *>(aligned_buffer),
                          static_cast<char *>(aligned_buffer) + BLOCK_SIZE));

    bytes_written += to_write;
    count -= to_write;
    lseek(fd, to_write, SEEK_CUR);
  }

  free(aligned_buffer);
  return bytes_written;
}

off_t lab2_lseek(int fd, off_t offset, int whence) {
  return lseek(fd, offset, whence);
}

int lab2_fsync(int fd) {
  cache.fsync(fd);
  return fsync(fd);
}