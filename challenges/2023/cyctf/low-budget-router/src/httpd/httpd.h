#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

// Server control functions

void serve_forever(const char *PORT);

// Client request

extern char *method, // "GET" or "POST"
    *uri,            // "/index.html" things before '?'
    *qs,             // "a=1&b=2"     things after  '?'
    *prot;           // "HTTP/1.1"

extern char *payload; // for POST
extern int payload_size;
extern int request_size;
extern char *buf;

char *request_header(const char *name);

// user shall implement this function
void renderHtml(char *file);
void redirect(char *location);
void serverError();
void setLogin(char state);
char getLogin();
void route();

// some interesting macro for `route()`
#define ROUTE_START()       if (0) {
#define ROUTE(METHOD,URI)   } else if (strcmp(URI,uri)==0&&strcmp(METHOD,method)==0) {
#define ROUTE_GET(URI)      ROUTE("GET", URI) 
#define ROUTE_POST(URI)     ROUTE("POST", URI) 
#define ROUTE_END()         } else serverError();

#endif