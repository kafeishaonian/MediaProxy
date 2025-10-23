//
// Created by Hongmingwei on 2025/10/22.
//

#ifndef MEDIAPROXY_JSONPARSER_H
#define MEDIAPROXY_JSONPARSER_H

#include <string>
#include <vector>
#include <functional>

using namespace std;

namespace PBase{

    class JsonParser {

    public:
        JsonParser() = delete;

        JsonParser(const string& file_name);

        ~JsonParser();

        int parse();

        bool optGetBool(const string& key, bool opt_value);

        int optGetInt(const string& key, int opt_value);

        int64_t optGetInt64(const string& key, int64_t opt_value);

        uint64_t optGetInt64(const string& key, uint64_t opt_value);

        string optGetString(const string& key, string opt_value);

        string optGetObject(const string& key);

        void enumObjectWithArray(function<void(const string subObjJson, bool& stop)> enum_func);

        // 解析StringArray
        int optGetStringArray(const string& key,
                              vector<string>& vector,
                              const bool check_item_is_number = false);

        int optGetIntArray(const string& key, vector<int>& vector);

        string ptrGetString(const string& pointer, string opt_value);

        int ptrGetInt(const string& pointer, int opt_value);

        int64_t ptrGetInt64(const string& pointer, int64_t opt_value);

        uint64_t ptrGetUint64(const string& pointer, uint64_t opt_value);

        int ptrGetObjectCount(const string& pointer);

    private:
        class Impl;
        unique_ptr<Impl> _impl;




    };
}


#endif //MEDIAPROXY_JSONPARSER_H
