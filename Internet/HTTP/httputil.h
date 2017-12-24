#ifndef	__HTTPUTIL_H__
#define	__HTTPUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket.h"
#include "w5500.h"

#include "util.h" 
#include "httpd.h"


unsigned char do_http(void);
unsigned char proc_http(SOCKET s, unsigned char* buf);
void cgi_ipconfig(st_http_request * http_request);

void make_cgi_response(unsigned int delay, char* url,char* cgi_response_buf);
#endif

