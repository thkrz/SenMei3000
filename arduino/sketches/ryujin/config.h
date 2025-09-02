#ifndef _CONFIG_H
#define _CONFIG_H

#define HOST "erdrutsch.com"
#define PATH "/station"
#define PORT 8000
#define USER_AGENT "Ryujin/1.0"

#define HTTP_TIMEOUT 15000
#define HTTP_MSG_LEN 10

#define MODEM_TIMEOUT 10000

#define SDI_TIMEOUT 3000
#define SDI_ERROR(i, s) (s += i; s += "ERROR\r\n")

#define BAT_LOW 11.9

#endif /* _CONFIG_H */
