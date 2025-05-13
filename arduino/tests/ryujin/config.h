#ifndef _CONFIG_H
#define _CONFIG_H

#define HOST "erdrutsch.com"
#define PATH "/station"
#define PORT 8000
#define USER_AGENT "Ryujin/1.0"

#define HTTP_TIMEOUT 15000
#define HTTP_MSG_LEN 10
#define HTTP_OK(x,n) ((n)>=10&&strncmp("HTTP/1.1 2",(x),10)==0)

#define SDI_TIMEOUT 3000

#define BAT_LOW 11.4

#endif // _CONFIG_H
