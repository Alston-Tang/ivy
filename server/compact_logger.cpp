//
// Created by Tang on 12/24/2018.
//

#include "compact_logger.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <chrono>
#include <string>
#include <iostream>

namespace ivy {

const static int FILE_MODE = ((mode_t) 0600);
const static int DIRECTORY_MODE = ((mode_t) 0700);
// Do not change this if some logging files has already been generated
// 4MB
const static size_t FILE_LENGTH = 4 * 1024 * 1024;
static unsigned int ZERO = 0;

bool CompactLogger::is_initialized() {
    return len && map_begin && map_end && cur_ptr;
}

bool CompactLogger::log(char *msg, size_t msg_len) {
    if (!is_initialized()) return false;

    bool res;
    if (*len + msg_len > FILE_LENGTH - 4) {
        res = roll();
        if (!res) {
            return false;
        }
    }

    // Should always false
    if (*len + msg_len > FILE_LENGTH - 4) {
        LOG(ERROR) << "We still don't have enough space to log. What's going on here?";
        return false;
    }

    memcpy(cur_ptr, msg, msg_len);
    cur_ptr = (uint8_t *) cur_ptr + msg_len;
    *len += msg_len;

    sync();
    return true;
}

CompactLogger::CompactLogger(std::string logger_name_, long latest_start_time_, std::string logging_path) {
    LOG(INFO) << "Initilize logger " << logger_name_;
    logger_name = std::move(logger_name_);
    category_path = logging_path + "/" + logger_name;

    len = &ZERO;
    map_begin = nullptr;
    map_end = nullptr;
    cur_ptr = nullptr;

    int rv;

    if (latest_start_time_ < 0) {
        LOG(INFO) << "Cannot find any logging file. So try to create one.";
        if (!set_new_map()) {
            LOG(ERROR) << "Cannot create a new logging file.";
            clean();
        }
        return;
    }

    latest_start_time = latest_start_time_;
    std::string logging_file_name = category_path + "/" + "ivy_" + std::to_string(latest_start_time);
    int fd = open(logging_file_name.c_str(), O_RDWR, FILE_MODE);

    if (fd == -1) {
        LOG(ERROR) << "Cannot open logging file. This is an error.";
        perror("Error: open");
        return;
    }

    LOG(INFO) << "Load file into memory.";
    map_begin = mmap(nullptr, FILE_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_begin == MAP_FAILED) {
        LOG(ERROR) << "Cannot map file to memory.";
        perror("Error: mmap");
        map_begin = nullptr;
        map_end = nullptr;
        close(fd);
        return;
    }
    map_end = (uint8_t *) map_begin + FILE_LENGTH;

    // Read length. Length is a 4 byte unsigned integer at the first 4 byte of the file
    len = (unsigned int *) map_begin;
    // Sanity check
    if (*len > FILE_LENGTH - 4) {
        LOG(ERROR) << "Bad file length field.";
        clean();
        return;
    }
    // Move cur_ptr
    cur_ptr = (uint8_t *) map_begin + 4 + *len;
    LOG(INFO) << "Finish initialization of logger " << logger_name;
}

CompactLogger::~CompactLogger() {
    LOG(INFO) << "Logger " << logger_name << " will be destroyed.";
    clean();
}

int CompactLogger::get_size() {
    return *len;
}

bool CompactLogger::set_new_map() {
    clean();


    int rv;
    auto time_now = std::chrono::steady_clock::now();
    auto time_stamp_now = time_now.time_since_epoch();
    auto time_now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_stamp_now);

    latest_start_time = time_now_ms.count();
    std::string logging_file_name = category_path + "/" + "ivy_" + std::to_string(latest_start_time);
    LOG(INFO) << "Create a new file " << logging_file_name << ".";
    int fd = open(logging_file_name.c_str(), O_RDWR | O_CREAT, FILE_MODE);

    if (fd == -1) {
        LOG(ERROR) << "Cannot open logging file. This is an error.";
        perror("Error: open");
        clean();
        return false;
    }

    LOG(INFO) << "Resize file to " << FILE_LENGTH << ".";
    rv = posix_fallocate(fd, 0, FILE_LENGTH);
    if (rv != 0) {
        LOG(ERROR) << "Cannot resize file to " << FILE_LENGTH << ".";
        perror("Error: posix_fallocate");
        clean();
        return false;
    }

    LOG(INFO) << "Load file into memory.";
    map_begin = mmap(nullptr, FILE_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_begin == MAP_FAILED) {
        LOG(ERROR) << "Cannot map file to memory.";
        perror("Error: mmap");
        map_begin = nullptr;
        map_end = nullptr;
        clean();
        return false;
    }
    map_end = (uint8_t *) map_begin + FILE_LENGTH;

    // It is good to close file after mmap
    rv = close(fd);
    if (rv != 0) {
        LOG(WARNING) << "Cannot close file after successfully map file to memory.";
        perror("Error: close");
    }

    // Read length. Length is a 4 byte unsigned integer at the first 4 byte of the file
    len = (unsigned int *) map_begin;
    *len = 0;
    // Sanity check
    if (*len > FILE_LENGTH - 4) {
        LOG(ERROR) << "Bad file length field.";
        clean();
        return false;
    }
    // Move cur_ptr
    cur_ptr = (uint8_t *) map_begin + 4 + *len;

    return true;
}

bool CompactLogger::roll() {
    if (!set_new_map()) {
        clean();
        return false;
    }
    return true;
}

void CompactLogger::clean() {
    int rv;
    if (map_begin) {
        rv = munmap(map_begin, FILE_LENGTH);
        if (rv != 0) {
            LOG(WARNING) << "Cannot unmap logging area during clean.";
            perror("Error: munmap");
        }
        map_begin = nullptr;
        map_end = nullptr;
    }
    cur_ptr = nullptr;
    len = &ZERO;
}

bool CompactLogger::shouldSync() {
    return is_initialized();
}

void CompactLogger::sync() {
    if (!shouldSync()) return;

    int rv = msync(map_begin, FILE_LENGTH, MS_SYNC);
    if (rv != 0) {
        LOG(WARNING) << "Cannot sync memory back to file.";
        perror("Error: msync");
    } else {
        LOG(INFO) << "Sync memory with file.";
    }
}

CompactLoggerManager &CompactLoggerManager::get_instance() {
    static CompactLoggerManager instance;
    return instance;
}

std::shared_ptr<CompactLogger> CompactLoggerManager::get_logger(std::string logger_name) {
    if (loggers.count(logger_name)) {
        return loggers[logger_name];
    }

    int rv;
    LOG(INFO) << "Cannot find logger instance " << logger_name << ". So try to create logger.";
    // TODO Should use a better path join method. C++17 Filesystem library?
    std::string category_path = std::string() + logging_path + "/" + logger_name;
    DIR *category_dir = opendir(category_path.c_str());
    if (category_dir == nullptr) {
        LOG(INFO) << "Cannot open category directory at " << category_path << ". So try to create one.";
        rv = mkdir(category_path.c_str(), DIRECTORY_MODE); // NOLINT

        if (rv != 0) {
            LOG(ERROR) << "Cannot create directory at " << category_path << ".";
            return nullptr;
        }

        category_dir = opendir(category_path.c_str());
        if (category_dir == nullptr) {
            LOG(ERROR) << "Still cannot open category directory. Give up then.";
            return nullptr;
        }
    }

    long latest_start_time = -1;
    while (true) {
        auto *cur_logging_file = readdir(category_dir);
        if (!cur_logging_file) break;
        if (cur_logging_file->d_type != DT_REG) {
            continue;
        }

        std::string logging_file_name(cur_logging_file->d_name);
        if (logging_file_name.size() <= 4 || logging_file_name.substr(0, 4) != "ivy_") {
            LOG(WARNING) << "Find non-logging file " << logging_file_name << ".";
            continue;
        }

        auto logging_start_time_str = logging_file_name.substr(4, logging_file_name.size() - 4);
        long logging_start_time = std::stol(logging_start_time_str);

        latest_start_time = std::max(latest_start_time, logging_start_time);
    }
    auto ins = new CompactLogger(logger_name, latest_start_time, logging_path);
    std::shared_ptr<CompactLogger> instance(ins, CompactLoggerDeleter());
    if (!instance->is_initialized()) {
        LOG(ERROR) << "Loger is not initialized.";
        return nullptr;
    }

    loggers[logger_name] = instance;
    return instance;
}

bool CompactLoggerManager::close_logger(std::string logger_name) {
    LOG(INFO) << "Will close logger " << logger_name << ".";
    auto it = loggers.find(logger_name);
    if (it == loggers.end()) {
        return false;
    }
    loggers.erase(it);
    return true;
}

CompactLoggerManager::CompactLoggerManager() {
    logging_path = std::string(IVY_SERVER_LOGGER_PATH);
    LOG(INFO) << "CompactLoggerManager is initializing.";
    LOG(INFO) << "Logging path: " << logging_path;
    LOG(INFO) << "CompactLoggerManager is initialized.";
}

}
