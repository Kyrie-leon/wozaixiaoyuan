#pragma once
#include<iostream>
#include<sstream>
#include<strings.h>
#include<vector>
#include<unordered_map>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include"Log.hpp"

#define BACKLOG 5

class Sock
{
  public:
    static int Socket()
    {
      int sock = socket(AF_INET, SOCK_STREAM, 0);
      if(sock < 0)
      {
        LOG(Fatal,"socket creat error");
        exit(SocketErr);
      }
      return sock;
    }

    static void Bind(int lsock, int port)
    {
      //填充结构体协议
      struct sockaddr_in local;
      bzero(&local, sizeof(local));
      local.sin_family = AF_INET;
      local.sin_addr.s_addr = htonl(INADDR_ANY);
      local.sin_port = htons(port);
      if(bind(lsock, (struct sockaddr* )&local, sizeof(local)) < 0 )
      {
        LOG(Fatal, "bind error");
        exit(BindErr);
      }
    }

    static void Listen(int sock)
    {
      if(listen(sock, BACKLOG) < 0)
      {
        LOG(Fatal, "listen error");
        exit(ListenErr);
      }
    }

    static void SetSockOpt(int sock)
    {
      int opt = 1;
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    static int Accept(int lsock)
    {
      struct sockaddr_in peer;
      socklen_t len = sizeof(peer);
      int s = accept(lsock, (struct sockaddr *)&peer, &len);
      if(s < 0)
      {
        LOG(Warning, "accetp error");
      }
      
      return s;
    }

    static void Getline(int sock, std::string& line)
    {
      //换行情况：1.\n 2.\r\n 3.\r
      //按字符读取
      char c = 'X';
      while(c != '\n')
      {
        ssize_t s = recv(sock, &c, 1, 0);
        if(s > 0){
          if(c == '\r'){
            //判断下一个字符是否\n
            ssize_t ss = recv(sock, &c, 1, MSG_PEEK);
            if(ss > 0 && c == '\n'){
              //从缓冲区读出来
              recv(sock, &c ,1, 0);
            }
            else{
              c = '\n';
            }
          }
          //走到这里三种情况
          //1.常规字符
          //2.\r\n统一处理为\n
          //3.\n
          if(c != '\n'){
            line.push_back(c);
          }
        }
      }
    }
};
