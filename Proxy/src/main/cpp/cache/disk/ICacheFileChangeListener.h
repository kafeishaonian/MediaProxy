//
// Created by Hongmingwei on 2025/10/28.
//

#ifndef MEDIAPROXY_ICACHEFILECHANGELISTENER_H
#define MEDIAPROXY_ICACHEFILECHANGELISTENER_H

class ICacheFileChangeListener {
public:
    virtual void cache_removed(const std::list<std::string>& file_keys) = 0;

    virtual void cache_added(const std::string& file_key) = 0;
};

#endif //MEDIAPROXY_ICACHEFILECHANGELISTENER_H