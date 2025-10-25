//
// Created by Hongmingwei on 2025/10/20.
//

#ifndef MEDIAPROXY_GZIPUTIL_H
#define MEDIAPROXY_GZIPUTIL_H

#include <string>
#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace boost::iostreams;

namespace proxy {

    std::string gzipCompress(const std::string& data);

    std::string gzipDecompress(const std::string& data);

};


#endif //MEDIAPROXY_GZIPUTIL_H
