//
// Created by Tang on 12/24/2018.
//

#ifndef IVY_COMPACT_LOGGER_H
#define IVY_COMPACT_LOGGER_H

#include <dirent.h>

#include <glog/logging.h>

#include <unordered_map>
#include <string>
#include <algorithm>
#include <memory>

namespace ivy {

const static int MAX_PATH_LENGTH = 1024;


class CompactLoggerManager;

class CompactLoggerDeleter;

class CompactLogger {
public:
    CompactLogger() = delete;

    CompactLogger(CompactLogger &) = delete;

    bool is_initialized();

    bool log(char *msg, size_t len);

    int get_size();

private:
    CompactLogger(std::string logger_name, long latest_start_time, std::string logging_path);
    ~CompactLogger();

    bool set_new_map();

    bool roll();

    void clean();

    bool shouldSync();

    void sync();


    std::string logger_name;
    std::string file_name;
    long latest_start_time;
    void *map_begin;
    // Address right after the mapped area
    void *map_end;
    std::string category_path;
    // Exclude length field (first 4 bytes)
    uint32_t *len;
    void *cur_ptr;

    friend class CompactLoggerManager;

    friend class CompactLoggerDeleter;
};

class CompactLoggerDeleter {
public:
    void operator()(CompactLogger *p) { delete p; }
};

class CompactLoggerManager {
public:
    CompactLoggerManager(CompactLoggerManager &) = delete;

    static CompactLoggerManager &get_instance();

    std::shared_ptr<CompactLogger> get_logger(std::string logger_name);

    bool close_logger(std::string logger_name);

private:
    CompactLoggerManager();

    friend class CompactLogger;

    std::unordered_map<std::string, std::shared_ptr<CompactLogger>> loggers;
    std::string logging_path;
};

}

#endif //IVY_COMPACT_LOGGER_H
