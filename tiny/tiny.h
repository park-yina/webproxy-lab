static void doit(int fd);
static void read_requesthdrs(rio_t* rp);
static int parse_uri(char* uri, char* filename, char* cgiargs);
static void serve_static(int fd, char* filename, int filesize);
static void get_filetype(char* filename, char* filetype);
static void serve_dynamic(int fd, char* filename, char* cgiargs);
static void clienterror(int fd, char* cause, char* errnum, char* shortmsg,
    char* longmsg);
