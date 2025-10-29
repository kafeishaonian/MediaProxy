//
// Created by Hongmingwei on 2025/10/22.
//

#ifndef MEDIAPROXY_JSONPARSER_H
#define MEDIAPROXY_JSONPARSER_H

#include <string>
#include <vector>
#include <functional>

#define RAPIDJSON_NAMESPACE proxy::Json
#define RAPIDJSON_NAMESPACE_BEGIN namespace proxy{ namespace Json{
#define RAPIDJSON_NAMESPACE_END }}
#define RAPIDJSON_NAMESPACE_PREFIX  proxy::Json

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/pointer.h>


namespace proxy {

    class JsonParser {

    public:
        JsonParser() = delete;

        JsonParser(const std::string &file_name);

        ~JsonParser();

        int parse();

        bool opt_get_bool(const std::string &key, bool opt_value);

        int opt_get_int(const std::string &key, int opt_value);

        int64_t opt_get_int64(const std::string &key, int64_t opt_value);

        uint64_t opt_get_uint64(const std::string &key, uint64_t opt_value);

        std::string opt_get_string(const std::string &key, std::string opt_value);

        std::string opt_get_object(const std::string &key);

        void enum_object_with_array(
                std::function<void(const std::string sub_obj_json, bool &stop)>
                enum_func);

        int opt_get_string_array(const std::string &key,
                                 std::vector<std::string> &vector,
                                 const bool check_item_is_number = false);

        int opt_get_int_array(const std::string &key, std::vector<int> &vector);

        std::string ptr_get_string(const std::string &pointer, std::string opt_value);

        int ptr_get_int(const std::string &pointer, int opt_value);

        int64_t ptr_get_int64(const std::string &pointer, int64_t opt_value);

        uint64_t ptr_get_uint64(const std::string &pointer, uint64_t opt_value);

    private:
        class Impl {
        public:
            Impl(const std::string &json_string);

            int parse();

            bool opt_get_bool(const std::string &key, bool opt_value);

            int opt_get_int(const std::string &key, int opt_value);

            int64_t opt_get_int64(const std::string &key, int64_t opt_value);

            uint64_t opt_get_uint64(const std::string &key, uint64_t opt_value);

            std::string opt_get_string(const std::string &key, std::string opt_value);

            std::string opt_get_object(const std::string &key);

            void enum_object_with_array(
                    std::function<void(const std::string sub_obj_json, bool &stop)> enum_func);

            int opt_get_string_array(const std::string &key,
                                     std::vector<std::string> &vector,
                                     const bool check_item_is_number = false);

            int opt_get_int_array(const std::string &key, std::vector<int> &vector);

            std::string ptr_get_string(const std::string &pointer, std::string opt_value);

            int ptr_get_int(const std::string &pointer, int opt_value);

            int64_t ptr_get_int64(const std::string &pointer, int64_t opt_value);

            uint64_t ptr_get_uint64(const std::string &pointer, uint64_t opt_value);

        private:
            std::string json_string_;
            bool parsed_;
            RAPIDJSON_NAMESPACE_PREFIX::Document document_;

        };

        unique_ptr <Impl> impl_;

    };
}


#endif //MEDIAPROXY_JSONPARSER_H
