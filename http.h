#ifndef HTTP_H
#define HTTP_H 

#define BUFFER_SIZE 1024

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sstream>

#include <iostream>
#include <vector>
#include <string>

enum Method
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
};


class Http 
{
    protected: 
        int _socket;
        struct sockaddr_in _remote_server;
        Method _method;
        std::string _resource_path;
        std::string _hostname;
        std::vector<std::string> _headers;
        std::vector<std::string> _data;

    private:
        void get_host_by_url(const char* url);
        const std::string method_from_enum (Method method);
        bool startswith(std::string str, std::string substr);

    public:
        Http();
        Http(const char* url, const Method method = Method::GET);

        void set_remote_host(const char* url);
        void set_remote_port(const unsigned int port);
        void set_method(const Method method);

        void add_header(const std::string header);
        void set_data(const std::vector<std::string> data);

        char* get_remote_host() const;
        unsigned int get_remote_port() const;
        Method get_method() const;

        std::vector<std::string> get_headers() const;
        std::vector<std::string> get_data() const;

        // =================================================================== //

        std::string request(const bool auto_close);
        void close_socket();
};

#endif
