#pragma once
#include<iostream>
#include<string>
#include<sys/time.h>

#define Notice  1
#define Warning 2
#define Error   3
#define Fatal   4

#define LOG(level, message) \
  Log(#level, message, __FILE__, __LINE__)

enum ERR{
  SocketErr=1,
  BindErr,
  ListenErr,
  ArgErr
};


void Log(std::string level, std::string message, std::string filename, size_t line)
{
  struct timeval cur;
  gettimeofday(&cur, nullptr);
  std::cout << "["<< level <<"]" << "[" << message << "]" << "[" << cur.tv_sec << "]" << "[" << filename << "]" << "[" << line << "]" << std::endl;
}
