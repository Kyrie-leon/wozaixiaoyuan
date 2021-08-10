#pragma once
#include<unistd.h>
#include<pthread.h>
#include"Sock.hpp"
#include"Protocol.hpp"
#include"ThreadPool.hpp"

#define PORT 8081

class HttpServer
{
  private:
    int port;
    int lsock;
    ThreadPool *tp;
    static HttpServer *httpsvr;
    static pthread_mutex_t lock;
  public:
    //构造
    HttpServer(int _p = PORT)
      :port(_p),lsock(-1), tp(nullptr)
    {}
    
    static HttpServer* GetInstance(int port)
    {
      if(httpsvr == nullptr)
      {
        pthread_mutex_lock(&lock);
        if(httpsvr == nullptr)
        {
          httpsvr = new HttpServer(port);
        }
        pthread_mutex_unlock(&lock);
      }

      return httpsvr;
    }

    void InitServer()
    {
      signal(SIGPIPE, SIG_IGN);
      lsock = Sock::Socket();
      Sock::SetSockOpt(lsock);
      Sock::Bind(lsock, port);
      Sock::Listen(lsock);
      tp = new ThreadPool();
      tp->InitThreadPool();
    }
    
    void Start()
    {

      struct sockaddr_in endpoint;
      for(;;)
      {
        int sock = Sock::Accept(lsock);
        if(sock < 0)
        {
          continue;
        }
        LOG(Notice, "get a new link...");
        Task* tk = new Task(sock);
        tp->PushTask(tk);
       // pthread_t tid;
       // int *sockp = new int(sock);
       // pthread_create(&tid, nullptr, Entry::HandlerHttp, sockp);
       // pthread_detach(tid);
      }
    }

    //析构
    ~HttpServer()
    {
      if(lsock >= 0)
      {
        close(lsock);
      }
    }

};

HttpServer* HttpServer::httpsvr = nullptr;
pthread_mutex_t HttpServer::lock = PTHREAD_MUTEX_INITIALIZER;
