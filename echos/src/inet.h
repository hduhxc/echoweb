#ifndef _INET_H
#define _INET_H

#include "glib.h"
#include "gio/gio.h"

GSocket* ready_srv_socket(guint16 port);
GSocket* ready_cli_socket(gchar* ip_addr, guint16 port);
GSocket* acpt_socket(GSocket* sock);
gsize receive(GSocket* sock, gsize len, gchar* buf);
gboolean send_spec_len(GSocket* sock, gsize len, gchar* buf);
gboolean recv_spec_len(GSocket* sock, gsize len, gchar* buf);
void clo_socket(GSocket* sock);
gint get_socket_fd(GSocket* sock);
GIOChannel* socket_to_io_channel(gint sock_fd);

#endif
