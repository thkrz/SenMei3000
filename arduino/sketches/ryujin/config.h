#ifndef _CONFIG_H
#define _CONFIG_H

#define HOST "erdrutsch.com"
#define PATH "/station"
#define PORT 8000
#define USER_AGENT "Ryujin/1.0"

#define HTTP_TIMEOUT 15000
#define HTTP_MSG_LEN 10

#define SDI_TIMEOUT 3000
#define SDI_ERROR "SDI12_ERROR"
#define SDI_IS_ERROR(s) (s.indexOf("SDI12_ERROR")==-1)

#define BAT_LOW 11.9

#endif /* _CONFIG_H */
