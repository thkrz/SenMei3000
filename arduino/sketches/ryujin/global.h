#ifndef _GLOBAL_H
#define _GLOBAL_H

#define HOST "erdrutsch.com"
#define PATH "/station"
#define PORT 8000
#define USER_AGENT "Ryujin/1.0"

#define HTTP_TIMEOUT 15000
#define HTTP_MSG_LEN 12
#define HTTP_MSG_OK(x) (strncmp("HTTP/1.1 201",(x),12)==0)

#define SDI_TIMEOUT 3000

#define BAT_LOW 12.2

#endif // _GLOBAL_H
