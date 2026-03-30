#pragma once
#include <functional>
#include <memory>

struct IO_Request{
    int fd;
    size_t bytes_to_io;
    bool io_type;
    std::function<void(std::shared_ptr<std::vector<char>>)> callback;
    bool is_shutdown_flag=false;
    std::shared_ptr<std::vector<char>> buffer;
    IO_Request* next;
    IO_Request* prev;
};
