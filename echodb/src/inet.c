#include <string.h>
#include "inet.h"

#define _handle_error(err)	do { \
								if (err != NULL) { \
									g_error("%s,%d: %d %s", __FILE__, __LINE__, err->code, err->message); \
								} \
							} while(0)

GSocket* ready_srv_socket(guint16 port)
{
	GSocket* sock;
	GSocketAddress* sock_addr;
	GInetAddress* inet_addr;
	GError* err = NULL;

	sock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, (GSocketProtocol)0, &err);
	_handle_error(err);
	inet_addr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
	sock_addr = g_inet_socket_address_new(inet_addr, port);
	
	g_socket_bind(sock, sock_addr, TRUE, &err);
	_handle_error(err);
	g_socket_listen(sock, &err);
	_handle_error(err);
	g_message("Socket ready, Listening on port %d", port);

	return sock;
}

GSocket* ready_cli_socket(gchar* ip_addr,
						  guint16 port)
{
	GSocket *sock;
	GSocketAddress* cli_sock_addr;
	GInetAddress* cli_inet_addr;
        GInetAddress* tmp_addr;
	GSocketAddress* srv_sock_addr;
	GError* err = NULL;

	sock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, (GSocketProtocol)0, &err);
	_handle_error(err);
	cli_inet_addr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
	cli_sock_addr = g_inet_socket_address_new(cli_inet_addr, 0);
	tmp_addr = g_inet_address_new_from_string(ip_addr);
	srv_sock_addr = g_inet_socket_address_new(tmp_addr, port);

	g_socket_bind(sock, cli_sock_addr, FALSE, &err);
	_handle_error(err);
	g_socket_connect(sock, srv_sock_addr, NULL, &err);
	_handle_error(err);

	return sock;
}

GSocket* acpt_socket(GSocket* sock)
{
	GError* err = NULL;
	GSocket* acpt_sock = g_socket_accept(sock, NULL, &err);
	_handle_error(err);

	return acpt_sock;
}

void recv_spec_len(GSocket* sock,
				   gsize len,
				   gchar* buf)
{
	gsize count = len;
	gsize nr;
	gchar* p = buf;
	GError* err = NULL;

	while (count > 0) {
		nr = g_socket_receive(sock, p, count, NULL, &err);
		// _handle_error(err);

		if (nr == 0) {
			memset(buf, 0, len);
			return;
		}
		count -= nr;
		p += nr;
	}
}

void send_spec_len(GSocket* sock,
				   gsize len,
				   gchar* buf)
{
	gsize count = 0;
	gssize nr;
	GError* err = NULL;

	while (count < len) {
		nr = g_socket_send(sock, buf, len - count, NULL, &err);
		// _handle_error(err);

		if (nr == 0) {
			clo_socket(sock);
			return;
		}
		count += nr;
		buf += nr;
	}
}

void clo_socket(GSocket* sock)
{
	GError* err = NULL;
	g_socket_close(sock, &err);
	_handle_error(err);
}

gint get_socket_fd(GSocket* sock)
{
	return g_socket_get_fd(sock);
}
