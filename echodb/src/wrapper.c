#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "wrapper.h"

#define _handle_error(err)	do { \
								if (err == -1) { \
									g_error("%s,%d: %d %s", __FILE__, __LINE__, errno, strerror(errno)); \
								} \
							} while(0)

gsize __filelength(gint fd)
{
#ifdef G_OS_WIN32
	return  _filelength(fd);
#else
	struct stat st;
	gssize code = fstat(fd, &st);
	
	_handle_error(code);
	return st.st_size;
#endif
}

gssize __pread(gint fd,
			   void* buf, gsize count, gsize pos)
{
#ifdef G_OS_WIN32
	_lseek(fd, pos, SEEK_SET);
	gssize nr = _read(fd, buf, count);
#else
	lseek(fd, pos, SEEK_SET);
	gssize nr = read(fd, buf, count);
#endif
	_handle_error(nr);
	return nr;
}

gssize __pwrite(gint fd,
			   const void* buf, gsize count, gsize pos)
{
#ifdef G_OS_WIN32
	_lseek(fd, pos, SEEK_SET);
	gssize nr = _write(fd, buf, count);
#else
	lseek(fd, pos, SEEK_SET);
	gssize nr = write(fd, buf, count);
#endif
	_handle_error(nr);
	return nr;
}

GIOChannel* socket_to_io_channel(gint sock_fd)
{
#ifdef G_OS_WIN32
	return g_io_channel_win32_new_socket(sock_fd);
#else
	return g_io_channel_unix_new(sock_fd);
#endif
}

gint __open(gchar* name)
{
#ifdef G_OS_WIN32
	gssize code = _sopen(name, _O_BINARY | _O_CREAT | _O_RDWR, _SH_DENYNO, _S_IWRITE);
#else
	gssize code = open(name, O_CREAT | O_RDWR, 777);
#endif
	_handle_error(code);
	return code;
}

gssize __close(gint fd)
{
#ifdef G_OS_WIN32
	gssize code = _close(fd);
#else
	gssize code = close(fd);
#endif
	_handle_error(code);
	return code;
}
