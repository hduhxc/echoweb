#ifndef MY_LIB_H
#define MY_LIB_H

char* strnstr(char* haystack, char *needle, int n);
char* strnchr(char* s, int c, int n);
char* strpstr(char* haystack, char* needle, char* en);
char* strpchr(char* s, int c, char* en);
int strpcmp(char* s1, char* s2, char* en);
char* strappend(char* first, char* second);

int Pipe(int* pfds);
int Read(int fd, void* buf, unsigned int count);
int Write(int fd, const void *buf, unsigned int count);
int Close(int fd);

int fd_send_spec_len(int fd, char* buf, unsigned int len);
int fd_recv_spec_len(int fd, char* buf, unsigned int len);

#endif /* end of include guard: MY_LIB_H */
