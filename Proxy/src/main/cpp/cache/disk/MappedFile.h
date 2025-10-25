//
// Created by Hongmingwei on 2025/10/25.
//

#ifndef MEDIAPROXY_MAPPEDFILE_H
#define MEDIAPROXY_MAPPEDFILE_H

#include <string>

class MappedFile {

public:
    MappedFile(const std::string &file_name, int64_t segment_size);

    ~MappedFile();

    void set_file_offset(int64_t offset);

    int64_t write_data(int64_t offset, uint8_t *buffer, int64_t size);

    int64_t read_data(int64_t offset, uint8_t* buffer, int64_t size);

    int64_t write_string_to_file(const std::string& content);

    std::string read_string_from_file();

private:
    int zero_file(int fd, int64_t start_post, int64_t size);

private:
    int file_handler_;
    std::string file_name_;
    int64_t segment_size_;
    int64_t file_offset_;
    void *segment_ptr_;
};

namespace proxy {
    int get_page_size();
}


#endif //MEDIAPROXY_MAPPEDFILE_H

