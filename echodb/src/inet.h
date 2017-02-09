#ifndef _INET_H
#define _INET_H

#include "glib.h"
#include "gio/gio.h"

GSocket* ready_srv_socket(guint16 port);
GSocket* ready_cli_socket(gchar* ip_addr, guint16 port);
GSocket* acpt_socket(GSocket* sock);
void send_spec_len(GSocket* sock, gsize len, gchar* buf);
void recv_spec_len(GSocket* sock, gsize len, gchar* buf);
void clo_socket(GSocket* sock);
gint get_socket_fd(GSocket* sock);

#endif
