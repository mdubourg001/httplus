# httplus
A simple C++ HTTP lib based on sockets.

#### Examples of use: 

HTTP GET request on [www.foobar.com?say=hello&to=mom](www.foobar.com?say=hello&to=mom) :
```cpp
#include "http.h"

// the default used port is the default http port: 80
Http *http = new Http("www.foobar.com?say=hello&to=mom", Method::GET);

string res = http->request(true); // request(bool : auto close the socket after requesting) 
```
<br/>

HTTP POST request on [www.foobar.com](www.foobar.com) with parameters:
```cpp
#include "http.h"

Http *http = new Http("www.foobar.com", Method::POST);
// give data in a std::vector with key=value format 
http->set_data({ "say=hello", "to=mom" });
string res = http->request(true); 
```
<br />

To add headers to your request:
```cpp
/* the Host header is added automatically at initialisation */
http->add_header("Connection: close");
http->add_header("Accept: application/json");
```
<br />

To change the used port (by default 80), just do:
```cpp
http->set_remote_port(4242);
```
<br />

To set parameters after object construction:
```cpp
Http *http = new Http();
http->set_method(Method::DELETE);
http->set_remote_host("www.foobar.com");
```





