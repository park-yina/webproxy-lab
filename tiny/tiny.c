/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"
#include "tiny.h"
//TODO: 컴파일 에러를 막기 위한 선언부입니다.
doit(int fd) {
    void doit(int fd) {
        int is_static;
        struct stat sbuf;
        char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
        char filename[MAXLINE], cgiargs[MAXLINE];
        rio_t rio;

        /* 요청 라인 읽기 */
        Rio_readinitb(&rio, fd);
        if (!Rio_readlineb(&rio, buf, MAXLINE))
            return;

        /* 요청 라인 출력 */
        printf("%s", buf);

        /* 요청 라인 파싱 */
        sscanf(buf, "%s %s %s", method, uri, version);
        if (strcasecmp(method, "GET")) {
            /* 메소드가 GET이 아닌 경우 오류 메시지 제공 */
            clienterror(fd, method, "501", "Not Implemented", "Tiny는 이 메소드를 구현하지 않습니다.");
            return;
        }

        /* 요청 헤더 읽기 */
        read_requesthdrs(&rio);

        /* GET 요청에서 URI 파싱 */
        is_static = parse_uri(uri, filename, cgiargs);
        if (stat(filename, &sbuf) < 0) {
            /* 파일을 찾을 수 없는 경우 오류 메시지 제공 */
            clienterror(fd, filename, "404", "Not found", "Tiny가 이 파일을 찾을 수 없습니다.");
            return;
        }

        if (is_static) { /* 정적 컨텐츠 제공 */
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
                /* 파일을 읽을 수 없는 경우 오류 메시지 제공 */
                clienterror(fd, filename, "403", "Forbidden", "Tiny가 이 파일을 읽을 수 없습니다.");
                return;
            }
            serve_static(fd, filename, sbuf.st_size);
        }
        else { /* 동적 컨텐츠 제공 */
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
                /* CGI 프로그램을 실행할 수 없는 경우 오류 메시지 제공 */
                clienterror(fd, filename, "403", "Forbidden", "Tiny가 이 CGI 프로그램을 실행할 수 없습니다.");
                return;
            }
            serve_dynamic(fd, filename, cgiargs);
        }
    }

}

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}
