#pragma once
#include"Sock.hpp"
#include"Log.hpp"
#include"Util.hpp"

#define WEBROOT "wwwroot"
#define HOMEPAGE "index.html"
#define VERSION "HTTP/1.0"

static std::string CodeToDesc(int code)
{
  std::string desc;
  switch(code){
    case 200:
      desc = "OK";
      break;
    case 404:
      desc = "Not Found";
      break;
    default:
      desc = "OK";
      break;
    }
  return desc;
}

static std::string SuffixToDesc(const std::string &suffix)
{
  if(suffix == ".html" || suffix == ".htm"){
    return "text/html";
  }
  else if(suffix == ".js"){
    return "applicaton/x-javascript";
  }
  else if(suffix == ".css"){
    return "text/css";            
  }
  else if(suffix == ".jpg"){
    return "image/jpeg";            
  }
  else{
    return "text/html";            
  }
}


//http请求类
class HttpRequest{
  private:
    std::string request_line; //请求行
    std::string method;
    std::string uri;
    std::string version;
    std::vector<std::string> request_header;  //请求头
    ssize_t content_length;
    std::unordered_map<std::string, std::string> header_kv;
    std::string blank;  //空行
    std::string request_body; //请求体
    std::string path; //web路径
    std::string query_string;
    bool cgi;
    ssize_t file_size;
    std::string suffix;

  public:
    HttpRequest()
      :blank("\n"),content_length(-1),path(WEBROOT),cgi(false), suffix("text/html")
    {}
    void SetRequestLine(const std::string line)
    {
      request_line = line;
    }

    void RequestLineParse()
    {
      //拆分method uri 和version
      Util::StringParse(request_line, method, uri, version);
     // LOG(Notice, request_line);
     // LOG(Notice, method);
     // LOG(Notice, uri);
     // LOG(Notice, version);

    }
    void InsertHeaderLine(const std::string &line)
    {
      request_header.push_back(line);
      LOG(Notice, line);
    }
    
    void RequestHeaderParse()
    {
      //k,v方式存储key:value
      for(auto& e:request_header)
      {
        std::string k,v;
        Util::MakeStringToKV(e, k, v);
        LOG(Notice, k);
        LOG(Notice, v);
        //content-length转为int单独存储，方便读取正文
        if(k == "Content-Length"){
          content_length = Util::StringToInt(v);
        }
        header_kv.insert({k,v});
      }
    }

    bool IsNeedRecvBody()
    {
      //Post PoSt, POST需要不区分大小写
      if(strcasecmp(method.c_str(), "post") == 0 && content_length > 0)
      {
        return true;
      }
      return false;
    }

    ssize_t GetContenLength()
    {
      return content_length;
    }

    void SetRequestBody(std::string body)
    {
      request_body = body;
    }
    
    bool IsMethodLegal()
    {
      if(strcasecmp(method.c_str(), "POST") == 0 || strcasecmp(method.c_str(), "GET") == 0){
        return true;
      }
      return false;
    }

    bool IsGet()
    {
      return strcasecmp(method.c_str(), "GET")==0 ? true :false;
    }
    
    bool IsPost()
    {
      return strcasecmp(method.c_str(), "POST")==0 ? true :false;
    }
    
    bool IsCgi()
    {
      return cgi;
    }

    void SetCgi()
    {
      cgi = true;
    }
    void UriParse()
    {
      //uri存在
      std::size_t pos = uri.find("?");
      //通过？分离文件路径和请求参数
      if(pos == std::string::npos){
        path += uri;
      }
      else{
        //?左边代表文件路径 右边请求参数 同时设置cgi为true
        path += uri.substr(0, pos);
        query_string = uri.substr(pos+1);
        cgi = true;
      }
    }

    void SetUriEqPath()
    {
      path += uri;
    }

    void IsAddHomePage()
    {
      //加上根目录
      //path  or  /a/b/c.htm  or /s
      if(path[path.size()-1] == '/'){
        path += HOMEPAGE;
      }
    }
    
    std::string GetPath()
    {
      return path;
    }
    
    std::string SetPath(std::string _path)
    {
      path = _path;
    }
    

    void SetFileSize(ssize_t s)
    {
      file_size = s;
    }
    
    ssize_t GetFileSize()
    {
      return file_size;
    }

    std::string GetMethod()
    {
      return method;
    }

    std::string GetQueryString()
    {
      return query_string;
    }

    std::string GetBody()
    {
      return request_body;
    }

    std::string MakeSuffix()
    {
      //从后往前找到.划分后缀
      std::string suffix;
      std::size_t pos = path.rfind(".");
      if(pos != std::string::npos)
      {
        suffix = path.substr(pos);
      }
      return suffix;
    }

    ~HttpRequest()
    {}
};

//http响应类
class HttpResponse{
  private:
    std::string status_line;
    std::vector<std::string> response_header;
    std::string blank;
    std::string response_body;
  public:
    HttpResponse():blank("\n")
    {}
    
    void SetStatusLine(const std::string& line)
    {
      status_line = line;
    }

    std::string GetStatusLine()
    {
      return status_line;
    }

    std::vector<std::string> GetRspHeader()
    {
      response_header.push_back(blank);
      return response_header;
    }

    void AddHeader(const std::string &ct){
      response_header.push_back(ct);
    }

    ~HttpResponse()
    {}
};

class EndPoint{
  private:
    int sock; //套接字文件描述符
    HttpRequest req;
    HttpResponse rsp;
  private:
    void GetRequestLine()
    {
      std::string line;
      Sock::Getline(sock, line);  //获取请求行
      req.SetRequestLine(line);   //设置请求行
      req.RequestLineParse();     //分析请求行

    }
    void GetRequestHeader()
    {
      //Http请求按行读取存储
      std::string line;
      do{
          line = "";
          Sock::Getline(sock, line);
          req.InsertHeaderLine(line); //每一行放入vector
      }while(!line.empty());
      //请求头分离
      req.RequestHeaderParse();
    }

    void GetRequestBody()
    {
      //通过conten-length读取正文
      ssize_t len = req.GetContenLength();
      char c;
      std::string body;
      //将len个字节长度的内容读到body中,按字节读取
      while(len--)
      {
        ssize_t s = recv(sock, &c, 1, 0);
        body.push_back(c);
      }
      //设置请求正文
      req.SetRequestBody(body);
    }
  public:
    EndPoint(int _sock):sock(_sock)
    {}

    void RecvRequest()
    {
      //获取完整http请求
      //将http报文分解为报头，请求体，正文
      //读取并分析第一行
      GetRequestLine();
      //读取报头
      GetRequestHeader();
      //是否需要读取正文
      if(req.IsNeedRecvBody())
      {
        GetRequestBody();
      }
      //到这里读完了所有请求
    }

    void SetResponseStatusLine(int code)
    {

      std::string status_line;
      status_line += VERSION;
      status_line += " ";
      status_line += std::to_string(code);
      status_line += " ";
      status_line += CodeToDesc(code);
      status_line += "\r\n";
      
      rsp.SetStatusLine(status_line);
    }

    void SetResponseHeaderLine()
    {
      //根据后缀制作content_type
      std::string suffix = req.MakeSuffix();
      std::string content_type = "Content-Type: ";
      content_type += SuffixToDesc(suffix);
      content_type += "\r\n";
      rsp.AddHeader(content_type);
    }

    void MakeResponse()
    {
      //分析http请求
      //GET POST
      int code = 200;   //默认编码200
      ssize_t size = 0; //文件大小
      std::string path; //文件路径
      //请求方法不合法
      if(!req.IsMethodLegal())
      {
        LOG(Warning, "method is illegal");
        code = 404;
        goto end;
      }
      if(req.IsGet())
      {
        //GEt方法分析uri参数
        req.UriParse();
      }
      else{
        //Post方法,直接拼接
        req.SetUriEqPath();
      }
      req.IsAddHomePage();
      //get && 没有参数 ->path
      //get && 有参数 -> path && query_string
      //Post -> uri ->path && body
      path = req.GetPath();
      //LOG(Notice, path);
      struct stat st;
      //stat判断文件权限
      /**************************
       *返回值>0 存在
       *      <0 不存在
       *      
       *
       * *************************/
      if(stat(path.c_str(), &st) < 0){
        LOG(Warning, "html is not exist!404");
        code =404;
        goto end;
      }
      else{
        if(S_ISDIR(st.st_mode)){
          path += "/";
          req.SetPath(path);
          req.IsAddHomePage();
        }
        else{
          if((st.st_mode & S_IXUSR) ||\
             (st.st_mode & S_IXGRP) ||\
             (st.st_mode & S_IXOTH)){
              req.SetCgi();
          }
          else{
            //正常的网页请求
          }
        }
        if(!req.IsCgi()){
          //设置文件大小
          req.SetFileSize(st.st_size);
        }
      }
end:
      //制作response
      SetResponseStatusLine(code);  //制作响应行
      SetResponseHeaderLine();      //制作响应报头
      //TODO
    }

    void ExecNonCgi(const std::string path)
    {
      //非cgi直接返回文件数据即可
      ssize_t size = req.GetFileSize();
      int fd = open(path.c_str(), O_RDONLY);
      if(fd < 0)
      {
        LOG(Error, "path is not exist bug!!");
        return;
      }
      sendfile(sock, fd, nullptr, size);
      close(fd);
    }

    void ExecCgi()
    {
      std::string content_length_env;
      std::string path = req.GetPath();
      std::string method = req.GetMethod();
      std::string method_env = "METHOD=";
      method_env += method;

      //LOG(Notice, method);
      std::string query_string;
      std::string query_string_env;

      //管道通信方式,站在被调用进程角度
      std::string body;
      //创建管道
      int pipe_in[2] = {0};
      int pipe_out[2] = {0};

      pipe(pipe_in);
      pipe(pipe_out);
      //method->method_env 传入环境变量
      putenv((char*)method_env.c_str());
      pid_t id =fork();

      if(id == 0){
        //子进程
        close(pipe_in[1]);
        close(pipe_out[0]);

        dup2(pipe_in[0], 0);
        dup2(pipe_out[1], 1);

        //通过环境变量给子进程->GET->query_string
        if(req.IsGet()){
          query_string = req.GetQueryString();
          query_string_env = "QUERY_STRING=";
          query_string_env += query_string;
          putenv((char*)query_string_env.c_str());
        }
        else if(req.IsPost()){
          //content_length_env = "CONTENT-LENGTH=";
          //content_length_env += std::to_string(req.GetContenLength());
          //LOG(Notice, content_length_env);
          //putenv((char*)content_length_env.c_str());
          std::string query_prama;
          ssize_t len = req.GetContenLength();
          char c = 'X';
          for(ssize_t i = 0; i < len; i++){
            read(0, &c, 1);
            query_prama.push_back(c);
          }
          query_string_env = "QUERY_STRING=";
          query_string_env += query_prama;
          putenv((char*)query_string_env.c_str());
        }
        else{
          //TODO
        }
        //execl替换程序
        //LOG(Notice, "path");
        //LOG(Notice, path.c_str());
        execl(path.c_str(), path.c_str(), nullptr);
        exit(0);
      }
      close(pipe_in[0]);
      close(pipe_out[1]);
      //父进程
      //通过管道传递给子进程
      char c = 'X';
      if(req.IsPost()){
        body = req.GetBody();
        //LOG(Notice, body);
        ssize_t i = 0;
        for(;i < body.size(); ++i){
          write(pipe_in[1], &body[i], 1);
        }
      }
      ssize_t s = 0;
      do{
        s = read(pipe_out[0], &c, 1);
        if(s > 0){
          send(sock, &c, 1, 0);                                                         
          std::cout << c;
        }                                        
      }while(s > 0);
      std::cout << std::endl;
      waitpid(id, nullptr, 0);
    }

    void SendResponse()
    {
      //状态行 响应报头 依次返回
      std::string line = rsp.GetStatusLine();
      send(sock, line.c_str(), line.size(), 0);
      const auto &header = rsp.GetRspHeader();
      auto it = header.begin();
      while(it != header.end())
      {
        send(sock, it->c_str(), it->size(), 0);
        it++;
      }
      
      //是否cgi
      if(req.IsCgi())
      {
        LOG(Notice, "use cgi model!");
        ExecCgi();
      }
      else{
        LOG(Notice, "use non-cgi model!");
        std::string  path = req.GetPath();
        ExecNonCgi(path);
      }
    }
};

//http
class Entry{
  public:
    //HandlerHttp处理函数
    // 1.sock解引用
    static void HandlerHttp(int sock)
    {

     // int sock = *(int*)arg;
     // delete (int*)arg;
//测试代码显示http请求
#ifdef DEBUG
    char request[10240];
    recv(sock, request, sizeof(request), 0);
    std::cout << request << std::endl;
    close(sock);
#else
    EndPoint* ep = new EndPoint(sock);
    ep->RecvRequest();  //接收远端http请求
    ep->MakeResponse(); //制作http响应
    ep->SendResponse(); //发送http响应
#endif
    }
};
