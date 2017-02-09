#ifndef _WRAPPER_H
#define _WRAPPER_H

#include "glib.h"

#ifdef G_OS_WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

gsize __filelength(gint fd);
gssize __pread(gint fd, void* buf, gsize count, gsize pos);
gssize __pwrite(gint fd, const void* buf, gsize count, gsize pos);
GIOChannel* socket_to_io_channel(gint sock_fd);
gint __open(gchar* name);
gssize __close(gint fd);

#endif
