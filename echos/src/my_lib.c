#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#include <errno.h>
#endif

#define _handle_error(err)	do { \
								if (err == -1) { \
									fprintf(stderr, "%s,%d: %d %s", __FILE__, __LINE__, errno, strerror(errno)); \
								} \
							} while(0)

char* strnstr(char* haystack, char *needle, int n)
{
	int i = 0;
	int needle_len = strlen(needle);

	for (i = 0; i < n - needle_len; i++) {
		if (strncmp(haystack + i, needle, needle_len) == 0) return haystack + i;
	}
	return NULL;
}

char* strnchr(char* s, int c, int n)
{
	int i = 0;
	for (i = 0; i < n; i++) {
		if (*(s + i) == c) return s + i;
	}
	return NULL;
}

char* strpstr(char* haystack, char* needle, char* en)
{
	int needle_len = strlen(needle);

	while (haystack <= en - needle_len + 1) {
		if (strncmp(haystack, needle, needle_len) == 0) return haystack;
		haystack++;
	}
	return NULL;
}

char* strpchr(char* haystack, int c, char* en)
{
	while (haystack <= en) {
		if (*haystack == c) return haystack;
		haystack++;
	} 
	return NULL;
}

int strpcmp(char* s1, char* s2, char* en)
{
	while (s1 <= en) {
		if (*s1 > *s2) return 1;
		if (*s1 < *s2) return -1;
		s1++;
		s2++;
	}
	return 0;
}


char* strappend(char* first, char* second)
{
	int len = strlen(first) + strlen(second) + 1;
	char* new_str = (char*)malloc(len);

	sprintf(new_str, "%s%s", first, second);

	return new_str;
}

int Pipe(int* pfds)
{
#ifdef WIN32
	return _pipe(pfds, 4096, 0);
#else
	return Pipe(pfds);
#endif
}

int Read(int fd,
		 void* buf,
		 unsigned int count)
{
#ifdef WIN32
	return _read(fd, buf, count);
#else
	return read(fd, buf, count);
#endif
}

int Write(int fd,
		  const void *buf,
		  unsigned int count)
{
#ifdef WIN32
	return _write(fd, buf, count);
#else
	return write(fd, buf, count);
#endif
}

int Open(const char* filename,
		 int oflag,
		 int pmode)
{
#ifdef WIN32
	return _open(filename, _O_RDONLY, _S_IREAD);
#else
	return open(filename, O_RDONLY, 777);
#endif // WIN32
}

int Close(int fd)
{
#ifdef WIN32
	return _close(fd);
#else
	return close(fd);
#endif
}

int fd_send_spec_len(int fd,
				     char* buf,
				     unsigned int len)
{
	int count = 0;

	while (count < len) {
		int nr = Write(fd, buf, len - count);
		_handle_error(nr);

		if (nr == 0) {
			return -1;
		}
		count += nr;
		buf += nr;
	}
	return 0;
}

int fd_recv_spec_len(int fd,
					 char* buf,
					 unsigned int len)
{
	int count = 0;

	while (count < len) {
		int nr = Read(fd, buf, len - count);
		if (nr == 0) {
			return -1;
		}
		count += nr;
		buf += nr;
	}
	return 0;
}
