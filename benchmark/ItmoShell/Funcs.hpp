#pragma once

#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include "PageCacheFS.hpp"

namespace monolith {

bool EmaSearchStr(const std::string &filePath, const std::string &substring,
                  size_t blockSize) {
  if (substring.empty() || blockSize == 0) {
    std::cerr << "Invalid substring or block size.\n";
    return false;
  }

  std::ifstream file(filePath, std::ios::in);
  if (!file.is_open()) {
    std::cerr << "Unable to open file: " << filePath << "\n";
    return false;
  }

  std::string buffer;
  buffer.reserve(blockSize + substring.size() - 1);

  std::string overlap;
  while (file) {
    char *tempBuffer = new char[blockSize];
    file.read(tempBuffer, blockSize);
    std::size_t bytesRead = file.gcount();

    if (bytesRead == 0) {
      delete[] tempBuffer;
      break;
    }

    buffer = overlap + std::string(tempBuffer, bytesRead);
    delete[] tempBuffer;

    if (buffer.find(substring) != std::string::npos) {
      return true;
    }

    overlap = buffer.substr(buffer.size() - substring.size() + 1);
  }

  return false;
}

bool DirectEmaSearchStr(const std::string &filePath,
                        const std::string &substring, size_t blockSize) {
  if (substring.empty() || blockSize == 0) {
    std::cerr << "Invalid substring or block size.\n";
    return false;
  }

  // Открываем файл с флагом O_DIRECT
  int fd = open(filePath.c_str(), O_RDONLY | O_DIRECT);
  if (fd < 0) {
    perror("Unable to open file");
    return false;
  }

  // Проверяем выравнивание и выделяем буфер
  void *alignedBuffer = nullptr;
  size_t alignment = 512; // Обычно 512 или 4096 байт
  size_t bufferSize = blockSize + substring.size() - 1;

  if (posix_memalign(&alignedBuffer, alignment, bufferSize) != 0) {
    std::cerr << "Failed to allocate aligned memory.\n";
    close(fd);
    return false;
  }

  size_t overlapSize = substring.size() - 1;
  std::string overlap;

  while (true) {
    ssize_t bytesRead = read(fd, alignedBuffer, blockSize);
    if (bytesRead < 0) {
      perror("Error reading file");
      free(alignedBuffer);
      close(fd);
      return false;
    }

    // Если больше нечего читать, выходим из цикла
    if (bytesRead == 0) {
      break;
    }

    // Создаем строку из текущего буфера
    std::string currentBuffer =
        overlap + std::string(static_cast<char *>(alignedBuffer), bytesRead);

    // Проверяем, содержится ли подстрока в буфере
    if (currentBuffer.find(substring) != std::string::npos) {
      free(alignedBuffer);
      close(fd);
      return true;
    }

    // Сохраняем перекрытие для следующей итерации
    overlap = currentBuffer.substr(currentBuffer.size() - overlapSize);
  }

  // Освобождаем ресурсы
  free(alignedBuffer);
  close(fd);
  return false;
}

bool CacheEmaSearchStr(const std::string &filePath,
                       const std::string &substring, size_t blockSize) {
  if (substring.empty() || blockSize == 0) {
    std::cerr << "Invalid substring or block size.\n";
    return false;
  }

  // std::ifstream file(filePath, std::ios::in);
  auto file = lab2_open(filePath.c_str());

  if (file == -1) {
    std::cerr << "Unable to open file: " << filePath << "\n";
    return false;
  }

  std::string buffer;
  buffer.reserve(blockSize + substring.size() - 1);

  std::string overlap;
  while (file) {
    char *tempBuffer = new char[blockSize];
    auto count = lab2_read(file, tempBuffer, blockSize);
    std::size_t bytesRead = count;

    if (bytesRead == 0) {
      delete[] tempBuffer;
      break;
    }

    buffer = overlap + std::string(tempBuffer, bytesRead);
    delete[] tempBuffer;

    if (buffer.find(substring) != std::string::npos) {
      return true;
    }

    overlap = buffer.substr(buffer.size() - substring.size() + 1);
  }

  return false;
}

} // namespace monolith
