//
// Created by Hongmingwei on 2025/10/22.
//

#include "JsonParser.h"
#include "Util.h"
#include "BaseUtil.h"

namespace proxy {

    JsonParser::Impl::Impl(const std::string &json_string) :
            json_string_(json_string), parsed_(false) {
    }

    int JsonParser::Impl::parse() {
        if (json_string_.empty()) {
            return -1;
        }
        RAPIDJSON_NAMESPACE_PREFIX::ParseResult result = document_.Parse(json_string_.c_str());
        if (!result) {
            return -1;
        }
        parsed_ = true;
        return 0;
    }

    bool JsonParser::Impl::opt_get_bool(const std::string &key, bool opt_value) {
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return opt_value;
        }

        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        return value.GetBool();
    }

    int JsonParser::Impl::opt_get_int(const std::string &key, int opt_value) {
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return opt_value;
        }
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        if (value.IsInt()) {
            return value.GetInt();
        } else if (value.IsString()) {
            std::string get_string = value.GetString();
            int get_value = proxy::string_to_int(get_string);
            return get_value;
        } else {
            return opt_value;
        }
    }

    int64_t JsonParser::Impl::opt_get_int64(const std::string &key, int64_t opt_value) {
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return opt_value;
        }
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        if (value.IsInt64()) {
            return value.GetInt64();
        } else if (value.IsString()) {
            std::string get_string = value.GetString();
            uint64_t get_value = proxy::string_to_int64(get_string);
            return get_value;
        } else {
            return opt_value;
        }
    }

    uint64_t JsonParser::Impl::opt_get_uint64(const std::string &key, uint64_t opt_value) {
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return opt_value;
        }
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        if (value.IsUint64()) {
            return value.GetUint64();
        } else if (value.IsString()) {
            std::string get_string = value.GetString();
            uint64_t get_value = proxy::string_to_uint64(get_string);
            return get_value;
        } else {
            return opt_value;
        }
    }

    std::string JsonParser::Impl::opt_get_string(const std::string &key, std::string opt_value) {
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return opt_value;
        }
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        if (value.IsString()) {
            return value.GetString();
        } else {
            return opt_value;
        }
    }

    std::string JsonParser::Impl::opt_get_object(const std::string &key) {
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return std::string("");
        }
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        if (value.IsObject()) {
            RAPIDJSON_NAMESPACE_PREFIX::StringBuffer buffer;
            RAPIDJSON_NAMESPACE_PREFIX::Writer<RAPIDJSON_NAMESPACE_PREFIX::StringBuffer> writer(
                    buffer);
            value.Accept(writer);
            return std::string(buffer.GetString());
        } else {
            return std::string("");
        }
    }

    void JsonParser::Impl::enum_object_with_array(
            std::function<void(const std::string, bool &)> enum_func) {
        RAPIDJSON_ASSERT(document_.IsArray());
        RAPIDJSON_NAMESPACE_PREFIX::Document::ValueIterator member = document_.Begin();
        bool stop = false;
        for (; !stop && member != document_.End(); ++member) {
            if (member->IsObject()) {
                RAPIDJSON_NAMESPACE_PREFIX::StringBuffer buffer;
                RAPIDJSON_NAMESPACE_PREFIX::Writer<RAPIDJSON_NAMESPACE_PREFIX::StringBuffer> writer(
                        buffer);
                member->Accept(writer);
                enum_func(buffer.GetString(), stop);
            }
        }
    }

    int
    JsonParser::Impl::opt_get_string_array(const std::string &key, std::vector<std::string> &vector,
                                           const bool check_item_is_number) {
        int result = -1;
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return result;
        }
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        if (value.IsArray()) {
            for (auto it = value.Begin(); it != value.End(); it++) {
                if (it->IsString()) {
                    if (check_item_is_number) {
                        if (proxy::is_number(it->GetString())) {
                            vector.push_back(it->GetString());
                        }
                    } else {
                        vector.push_back(it->GetString());
                    }
                    result = 0;
                } else {
                    result = -1;
                    break;
                }
            }
            return result;
        } else {
            return result;
        }
    }

    int JsonParser::Impl::opt_get_int_array(const std::string &key, std::vector<int> &vector) {
        int result = -1;
        if (!parsed_ || !document_.HasMember(key.c_str())) {
            return result;
        }

        RAPIDJSON_NAMESPACE_PREFIX::Value &value = document_[key.c_str()];
        if (value.IsArray()) {
            for (auto it = value.Begin(); it != value.End(); it++) {
                if (it->IsInt()) {
                    vector.push_back(it->GetInt());
                    result = 0;
                } else {
                    result = -1;
                    break;
                }
            }
            return result;
        } else {
            return result;
        }
    }

    std::string
    JsonParser::Impl::ptr_get_string(const std::string &pointer, std::string opt_value) {
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = RAPIDJSON_NAMESPACE_PREFIX::Pointer(
                pointer.c_str()).GetWithDefault(document_, opt_value.c_str());
        if (value.IsString()) {
            return std::string(value.GetString());
        } else {
            return opt_value;
        }
    }

    int JsonParser::Impl::ptr_get_int(const std::string &pointer, int opt_value) {
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = RAPIDJSON_NAMESPACE_PREFIX::Pointer(
                pointer.c_str()).GetWithDefault(document_, opt_value);
        if (value.IsInt()) {
            return value.GetInt();
        } else if (value.IsString()) {
            std::string get_string = value.GetString();
            int get_value = proxy::string_to_int(get_string);
            return get_value;
        } else {
            return opt_value;
        }
    }

    int64_t JsonParser::Impl::ptr_get_int64(const std::string &pointer, int64_t opt_value) {
        RAPIDJSON_NAMESPACE_PREFIX::Value &value = RAPIDJSON_NAMESPACE_PREFIX::Pointer(
                pointer.c_str()).GetWithDefault(document_, opt_value);
        if (value.IsInt64()) {
            return value.GetInt64();
        } else if (value.IsString()) {
            std::string get_string = value.GetString();
            uint64_t get_value = proxy::string_to_int64(get_string);
            return get_value;
        } else {
            return opt_value;
        }
    }

    uint64_t JsonParser::Impl::ptr_get_uint64(const std::string &pointer, uint64_t opt_value) {
            RAPIDJSON_NAMESPACE_PREFIX::Value &value = RAPIDJSON_NAMESPACE_PREFIX::Pointer(
                    pointer.c_str()).GetWithDefault(document_, opt_value);
        if (value.IsUint64()) {
            return value.GetUint64();
        } else if (value.IsString()) {
            std::string get_string = value.GetString();
            uint64_t get_value = proxy::string_to_uint64(get_string);
            return get_value;
        } else {
            return opt_value;
        }
    }

    JsonParser::JsonParser(const std::string &file_name) : impl_(new Impl(file_name)) {

    }

    JsonParser::~JsonParser() {}

    int JsonParser::parse() {
        return impl_->parse();
    }

    bool JsonParser::opt_get_bool(const std::string &key, bool opt_value) {
        return impl_->opt_get_bool(key, opt_value);
    }

    int JsonParser::opt_get_int(const std::string &key, int opt_value) {
        return impl_->opt_get_int(key, opt_value);
    }

    int64_t JsonParser::opt_get_int64(const std::string &key, int64_t opt_value) {
        return impl_->opt_get_int64(key, opt_value);
    }

    uint64_t JsonParser::opt_get_uint64(const std::string &key, uint64_t opt_value) {
        return impl_->opt_get_uint64(key, opt_value);
    }

    std::string JsonParser::opt_get_string(const std::string &key, std::string opt_value) {
        return impl_->opt_get_string(key, opt_value);
    }

    std::string JsonParser::opt_get_object(const std::string &key) {
        return impl_->opt_get_object(key);
    }

    void JsonParser::enum_object_with_array(std::function<void(const std::string, bool &)> enum_func) {
        return impl_->enum_object_with_array(enum_func);
    }


    int JsonParser::opt_get_string_array(const std::string &key, std::vector<std::string> &vector,
                                         const bool check_item_is_number) {
        return impl_->opt_get_string_array(key, vector, check_item_is_number);
    }

    int JsonParser::opt_get_int_array(const std::string &key, std::vector<int> &vector) {
        return impl_->opt_get_int_array(key, vector);
    }

    std::string JsonParser::ptr_get_string(const std::string &pointer, std::string opt_value) {
        return impl_->ptr_get_string(pointer, opt_value);
    }

    int JsonParser::ptr_get_int(const std::string &pointer, int opt_value) {
        return impl_->ptr_get_int(pointer, opt_value);
    }

    int64_t JsonParser::ptr_get_int64(const std::string &pointer, int64_t opt_value) {
        return impl_->ptr_get_int64(pointer, opt_value);
    }

    uint64_t JsonParser::ptr_get_uint64(const std::string &pointer, uint64_t opt_value) {
        return impl_->ptr_get_uint64(pointer, opt_value);
    }

}

