//
// Created by Hongmingwei on 2025/10/20.
//

#include "GzipUtil.h"

namespace PBase {


    std::string gzipCompress(const std::string& data) {
        std::stringstream compressed;
        std::stringstream origin(data);

        filtering_streambuf<input> out;
        out.push(gzip_compressor(gzip_params(gzip::best_compression)));
        out.push(origin);
        copy(out, compressed);

        return compressed.str();
    }

    std::string gzipDecompress(const std::string& data) {
        std::stringstream compressed(data);
        std::stringstream decompressed;

        filtering_streambuf<input> out;
        out.push(gzip_decompressor());
        out.push(compressed);
        copy(out, decompressed);

        return decompressed.str();
    }

}