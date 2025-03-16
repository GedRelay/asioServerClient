#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
int main()
{
    Json::Value root;
    root["id"] = 1001;
    root["data"] = "hello world";

    std::cout << typeid(root["id"]).name() << std::endl;
    std::cout << typeid(root["data"]).name() << std::endl;

    std::string request = root.toStyledString();
    std::cout << "request is " << request << std::endl;
    Json::Value root2;
    Json::Reader reader;
    reader.parse(request, root2);
    std::cout << "msg id is " << root2["id"] << " msg is " << root2["data"] << std::endl;
}