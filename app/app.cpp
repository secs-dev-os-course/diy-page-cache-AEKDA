#include <cstring>
#include <iostream>
#include "PageCacheFS.hpp"

int main() {
    const char* path = "data/testfile.txt";

    // Открытие файла
    int fd = lab2_open(path);
    if (fd == -1) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    // Запись данных в файл
    const char* writeData = "Hello, World!";
    if (lab2_write(fd, writeData, strlen(writeData)) == -1) {
        std::cerr << "Failed to write to file" << std::endl;
        return 1;
    }

    // Перемещение указателя на начало файла
    if (lab2_lseek(fd, 0, SEEK_SET) == -1) {
        std::cerr << "Failed to seek file" << std::endl;
        return 1;
    }

    // Чтение данных из файла
    char readData[50];
    if (lab2_read(fd, readData, sizeof(readData)) == -1) {
        std::cerr << "Failed to read from file" << std::endl;
        return 1;
    }
    std::cout << "Read from file: " << readData << std::endl;

    // Синхронизация данных с диском
    if (lab2_fsync(fd) == -1) {
        std::cerr << "Failed to sync file" << std::endl;
        return 1;
    }

    // Закрытие файла
    if (lab2_close(fd) == -1) {
        std::cerr << "Failed to close file" << std::endl;
        return 1;
    }

    return 0;
}
