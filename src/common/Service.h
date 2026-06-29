#ifndef SERVICE_H
#define SERVICE_H
#include <iostream>
#include <string.h>
class Service
{
public:
    virtual ~Service() = default;

    virtual std::string call(const std::string &method, const std::string& request) = 0;
};
#endif