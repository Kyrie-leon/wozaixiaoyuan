#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

void CalData(std::string &qs)
{
    //a=100&b=200
    std::string part1;
    std::string part2;
    int x = 0;
    int y = 0;
    std::size_t pos= qs.find("&");
    if(pos != std::string::npos){
        part1 = qs.substr(0, pos);
        part2 = qs.substr(pos+1);
    }

    pos = part1.find("=");
    if(pos != std::string::npos){
        x = atoi(part1.substr(pos+1).c_str());
    }
    pos = part2.find("=");
    if(pos != std::string::npos){
        y = atoi(part2.substr(pos+1).c_str());
    }

    std::cout << "<html>" << std::endl;
    std::cout <<"<h1>"<< x << " + " << y << " = " << x + y << "</h1>"<< std::endl;
    std::cout << x << " - " << y << " = " << x - y << std::endl;
    std::cout << x << " * " << y << " = " << x * y << std::endl;
    std::cout << x << " / " << y << " = " << x / y << std::endl;
    std::cout << "</html>" << std::endl;
}
int main()
{
    std::string method;
    std::string query_string;
    if(getenv("METHOD")){
        method = getenv("METHOD");
    }
    else{
        return 1;
    }

    if(strcasecmp(method.c_str(), "GET") == 0){
        query_string = getenv("QUERY_STRING");
        std::cout << query_string << std::endl;
    }
    else if(strcasecmp(method.c_str(), "POST") == 0){
        //POST
        //cgi -> 0 -> 读取
        //cgi -> 1 -> 写入
        int cl = atoi(getenv("CONTENT-LENGTH"));
        std::cout << "hello: " << cl << std::endl;
        char c = 0;
        while(cl){
            read(0, &c, 1);
            query_string.push_back(c);
            cl--;
        }
        std::cout << "hello2: " << query_string << std::endl;
    }

    //query_string;
    //std::cout <<"use cgi # "<< query_string << std::endl;
    CalData(query_string);
    return 0;
}
