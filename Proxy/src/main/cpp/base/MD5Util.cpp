//
// Created by Hongmingwei on 2025/10/20.
//

#include "MD5Util.h"

namespace PBase {

    MD5Util::Impl::Impl() {
        _context = av_md5_alloc();
        av_md5_init(_context);
    }

    MD5Util::Impl::~Impl() {
        if (_context) {
            av_freep(&_context);
        }
    }

    void MD5Util::Impl::update(const uint8_t *src, int len) {
        av_md5_update(_context, src, len);
    }

    std::string MD5Util::Impl::getResult() {
        uint8_t buffer[16];
        memset(buffer, 0, 16);
        av_md5_final(_context, buffer);

        char dst[40];
        memset(dst, 0, 40);
        int i = 0;
        for (i = 0; i < 16; ++i) {
            uint8_t value = buffer[i] >> 4;
            if (value < 0x0a) {
                dst[i * 2] = '0' + value;
            } else {
                dst[i * 2] = 'A' + value - 0x0a;
            }
            value = buffer[i] & 0x0f;

            if (value < 0x0f) {
                dst[i * 2 + 1] = '0' + value;
            } else {
                dst[i * 2 + 1] = 'A' + value - 0x0a;
            }
        }
        dst[i * 2] = '\0';
        std::string result = std::string(dst);
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    MD5Util::MD5Util() : _impl(new Impl()) {
    }

    MD5Util::~MD5Util() {

    }

    void MD5Util::update(const uint8_t *src, int len) {
        _impl->update(src, len);
    }

    std::string MD5Util::getResult() {
        return _impl->getResult();
    }
}