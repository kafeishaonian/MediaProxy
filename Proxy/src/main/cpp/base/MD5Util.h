//
// Created by Hongmingwei on 2025/10/20.
//

#ifndef MEDIAPROXY_MD5UTIL_H
#define MEDIAPROXY_MD5UTIL_H

#include <memory>
#include <string>

#include <libavutil/md5.h>
#include <libavutil/base64.h>
#include <libavutil/avutil.h>

#include <algorithm>


namespace PBase {

    class MD5Util {

    public:
        MD5Util();

        ~MD5Util();

        void update(const uint8_t *src, int len);

        std::string getResult();


    private:
        class Impl {
        public:
            Impl();

            void update(const uint8_t *src, int len);

            std::string getResult();

        private:
            AVMD5* context_;

        public:
            ~Impl();
        };

        std::unique_ptr<Impl> impl_;
    };
}


#endif //MEDIAPROXY_MD5UTIL_H
