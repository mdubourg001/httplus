#include "http.h"

using namespace std;

Http::Http() 
    : _socket {socket(AF_INET, SOCK_STREAM, 0)},
      _method {Method::GET},
      _resource_path {"/"}
{
    this->_remote_server.sin_family = AF_INET;
    this->_headers = {};
}

Http::Http(const char* url, const Method method)
    : _socket {socket(AF_INET, SOCK_STREAM, 0)},
      _method {method},
      _resource_path {"/"}
{
    this->_remote_server.sin_family = AF_INET;
    this->_remote_server.sin_port = htons(80);

    this->get_host_by_url(url);

    string host_header = "Host: " + this->_hostname;
    this->_headers = {
        host_header
    };
}

// ======================================================================= //

void Http::set_remote_host(const char* url)
{
    this->get_host_by_url(url);
}

void Http::set_remote_port(const unsigned int port)
{
    this->_remote_server.sin_port = htons(port);
}

void Http::set_method(const Method method)
{
    this->_method = method;
}

void Http::add_header(const string header)
{
    string new_header_name = header.substr(0, header.find(':'));

    bool replace = false;
    for (unsigned int i = 0 ; i < this->_headers.size() ; i++)
    {
        string head_n = this->_headers.at(i).substr(0, this->_headers.at(i).find(':'));
        if (new_header_name == head_n)
        {
            this->_headers.at(i) = header;
            replace = true;
        }
    }

    if (!replace)
        this->_headers.push_back(header);
}

void Http::set_data(const vector<string> data)
{
    this->_data.clear();

    if (!data.empty())
        for (unsigned int i = 0 ; i < data.size() ; i++)
            this->_data.push_back(data.at(i));

    int data_len = 0;
    //adding length of the data + length of the & between each
    for (auto &p : this->_data)
        data_len += p.length() + 1;

    this->add_header("Content-Type: application/x-www-form-urlencoded");
    this->add_header("Content-Length: " + to_string(data_len - 1));
}

char* Http::get_remote_host() const
{
    puts("[TODO] Implement inet_add to string. Aborting.");
    return {0};
}

unsigned int Http::get_remote_port() const
{
    return (unsigned int)this->_remote_server.sin_port;
}

Method Http::get_method() const 
{
    return this->_method;
}

vector<string> Http::get_headers() const
{
    return this->_headers;
}

vector<string> Http::get_data() const
{
    return this->_data;
}

// ======================================================================= //

string Http::request(const bool auto_close)
{
    char buffer[BUFFER_SIZE] = { 0 };
    string response = "";

    if (connect(this->_socket,
                (struct sockaddr *)&this->_remote_server,
                sizeof(this->_remote_server)) < 0)
    {
        cout << "Connection Failed" << endl;
        exit(1);
    }

    //adding request line
    string http_request = method_from_enum(this->_method) + " "
            + this->_resource_path +" HTTP/1.1\r\n";

    //adding headers
    for (const auto& h : this->_headers)
        http_request += h + "\r\n";

    //adding blank line
    http_request += "\r\n";

    //adding body data
    for (unsigned int i = 0 ; i < this->_data.size() ; i++)
    {
        http_request += this->_data.at(i);
        if (i < this->_data.size() - 1)
            http_request += "&";
    }

    send(this->_socket, http_request.c_str(), http_request.length(), 0);

    int bytes = 0;
    int content_length;
    bool found_length = false;
    unsigned int header_size = 0;

    bytes += recv(this->_socket, buffer, BUFFER_SIZE, 0);
    response.append(buffer);

    //trying to get the content-length header
    istringstream f(buffer);
    string line;
    while (getline (f, line))
    {
        //getting header size to get number of bytes to read
        header_size += line.length() - 1;

        if (startswith(line, "Content-Length"))
        {
            content_length = atoi(line.substr(line.find(':') + 1).c_str());
            found_length = true;
        }

        else if (line.empty() || line == "\r" || line == "\r\n")
        {
            bytes -= header_size;
            break;
        }
    }

    //if no Content-Length header, reading till the server closes the connection
    if (!found_length)
    {
        memset(&buffer[0], 0, sizeof(buffer));
        while (recv(this->_socket, buffer, BUFFER_SIZE, 0) > 0)
        {
            response.append(buffer);
            memset(&buffer[0], 0, sizeof(buffer));
        }
    }

    //else reading the right number of bytes
    else
    {
        while (bytes <= content_length)
        {
            memset(&buffer[0], 0, sizeof(buffer));
            bytes += recv(this->_socket, buffer, BUFFER_SIZE, 0);
            response.append(buffer);
        }
    }

    if (auto_close)
        this->close_socket();

    return response;
}


void Http::close_socket()
{
    close(this->_socket);
}

// ======================================================================= //

void Http::get_host_by_url(const char* url)
{
    if(strstr(url, "https://") != NULL) {
        url += sizeof("https://")-1;
        cout << "HTTPS not yet supported." << endl;
        exit(1);
    }

    else if(strstr(url, "http://") != NULL)
	url += sizeof("http://")-1;

    const char *search = strstr(url, "/");

    if(search != NULL) {
        char *host = new char[strlen(url + (search-url))+1];
        strncpy(host, url, search-url);
        this->_hostname = string(host);
        delete host;
        this->_resource_path = string(search);
    }

    else
        this->_hostname = string(url);

    if (inet_pton(AF_INET, this->_hostname.c_str(), &(this->_remote_server.sin_addr)) < 1)
    {
        struct addrinfo hints, *servinfo, *p;
        int status = 0;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo(this->_hostname.c_str(), "http", &hints, &servinfo)) != 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            exit(1);
        }

        char ipstr[INET6_ADDRSTRLEN];

        for(p = servinfo ; p != NULL ; p = p->ai_next) {
            void *addr;

            if (p->ai_family == AF_INET) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                addr = &(ipv4->sin_addr);
            } else {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                addr = &(ipv6->sin6_addr);
            }

            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
            inet_pton(AF_INET, ipstr, &(this->_remote_server.sin_addr));
        }
    }
}


const string Http::method_from_enum (Method method)
{
    switch (method)
    {
        case GET:		return "GET";
        case HEAD:		return "HEAD";
        case POST:		return "POST";
        case PUT:		return "PUT";
        case DELETE:	return "DELETE";
        case OPTIONS:	return "OPTIONS";
        case TRACE:		return "TRACE";
        default:		return "GET";
    }
}


bool Http::startswith(string str, string substr)
{
    if (str.length() < 1 || substr.length() < 1)
        return false;


    for (unsigned int i = 0 ; i < substr.size() ; i++)
        if (str.at(i) != substr.at(i))
            return false;

    return true;
}

