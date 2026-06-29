#ifndef MY_SERVICE
#define MY_SERVICE

#include "../common/Service.h"
#include <unordered_map>
#include <functional>
class UserService : public Service
{

public:
    UserService()
    {
        m_methods["queryAge"] =
            [this](const std::string& req)
            {
                return queryAge(req);
            };

        m_methods["queryName"] =
            [this](const std::string& req)
            {
                return queryName(req);
            };
    }

    ~ UserService()
    {
        
    }

    std::string call(const std::string &method, const std::string &request) override
    {
         auto it = m_methods.find(method);

        if(it == m_methods.end())
        {
            return "method not found";
        }

        return it->second(request);
    }

private:
    std::string queryAge(
        const std::string &id)
    {
        return "25";
    }

    std::string queryName(
        const std::string &id)
    {
        return "zhangsan";
    }

    std::unordered_map<std::string,std::function<std::string(const std::string&)>> m_methods;
};

class OrderService : public Service
{
public:

    OrderService()
    {
        m_methods["queryOrder"] =
            [this](const std::string& req)
            {
                return queryOrder(req);
            };
    }

    std::string call(const std::string &method, const std::string &request) override
    {
         auto it = m_methods.find(method);

        if(it == m_methods.end())
        {
            return "method not found";
        }

        return it->second(request);
    }

private:

    std::string queryOrder(
        const std::string& id)
    {
        return "order_123";
    }

    std::unordered_map<std::string,std::function<std::string(const std::string&)>> m_methods;
};

#endif