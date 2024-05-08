#include <stdio.h>
#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

static const char* user_agent_hdr =
"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
"Firefox/10.0.3\r\n";
static const char* new_version = "HTTP/1.0";

void do_it(int fd);
void do_request(int clientfd, char* method, char* uri_ptos, char* host);
void do_response(int connfd, int clientfd);
int parse_uri(char* uri, char* uri_ptos, char* host, char* port);
void* thread(void* vargp);

int main(int argc, char** argv) {
    int listenfd, * connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

void* thread(void* vargp) {
    int connfd = *((int*)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    do_it(connfd);
    Close(connfd);
    return NULL;
}

void do_it(int connfd) {
    int clientfd;
    char buf[MAXLINE], host[MAXLINE], port[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char uri_ptos[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    parse_uri(uri, uri_ptos, host, port);

    clientfd = Open_clientfd(host, port);
    do_request(clientfd, method, uri_ptos, host);
    do_response(connfd, clientfd);
    Close(clientfd);
}

void do_request(int clientfd, char* method, char* uri_ptos, char* host) {
    char buf[MAXLINE];

    sprintf(buf, "GET %s %s\r\n", uri_ptos, new_version);
    sprintf(buf, "%sHost: %s\r\n", buf, host);
    sprintf(buf, "%s%s", buf, user_agent_hdr);
    sprintf(buf, "%sConnections: close\r\n", buf);
    sprintf(buf, "%sProxy-Connection: close\r\n\r\n", buf);

    Rio_writen(clientfd, buf, (size_t)strlen(buf));
}

void do_response(int connfd, int clientfd) {
    char buf[MAX_CACHE_SIZE];
    ssize_t n;
    rio_t rio;

    Rio_readinitb(&rio, clientfd);
    n = Rio_readnb(&rio, buf, MAX_CACHE_SIZE);
    Rio_writen(connfd, buf, n);
}

int parse_uri(char* uri, char* uri_ptos, char* host, char* port) {
    char* ptr;

    if (!(ptr = strstr(uri, "://")))
        return -1;

    ptr += 3;
    strcpy(host, ptr);

    if ((ptr = strchr(host, '/'))) {
        *ptr = '\0';
        ptr += 1;
        strcpy(uri_ptos, "/");
        strcat(uri_ptos, ptr);
    }
    else strcpy(uri_ptos, "/");

    if ((ptr = strchr(host, ':'))) {
        *ptr = '\0';
        ptr += 1;
        strcpy(port, ptr);
    }
    else strcpy(port, "80");

    return 0;
}
