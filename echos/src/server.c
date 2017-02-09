#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "inet.h"
#include "my_lib.h"
#include "config.h"
#include "service.h"

#ifdef G_OS_UNIX
#include <unistd.h>
#include <errno.h>
#endif

GMainLoop* loop;
GThreadPool* pool;
GSocket* listen_sock;

#define _handle_error(err)	do { \
								if (err != NULL) { \
									g_error("%s,%d: %d %s", __FILE__, __LINE__, err->code, err->message); \
								} \
							} while(0)

gssize read_line(gchar** p_buf_pointer,
				 gchar** buf_pointer,
			     GSocket* sock)
{
	gsize nr;
	gchar* p_buf = *p_buf_pointer;
	gchar* buf = *buf_pointer;

	if (p_buf == buf) {
		nr = receive(sock, MAX_LEN, buf);
		if (nr == 0) {
			clo_socket(sock);
			return -1;
		}
	}
	gsize pos = p_buf - buf;
	gsize len = (pos / MAX_LEN + 1) * MAX_LEN;
	gchar* p = strpstr(p_buf, "\r\n", buf + len - 1);

	if (p == NULL) {
		buf = g_realloc(buf, len + MAX_LEN);
		p_buf = buf + len;

		gsize nr = receive(sock, MAX_LEN, buf);
		if (nr == 0) {
			clo_socket(sock);
			return -1;
		}

		p = strpstr(p_buf, "\r\n", buf + len);
	}
	p += 2;
	
	*p_buf_pointer = p_buf;
	*buf_pointer = buf;
	return p - p_buf;
}

gchar* read_body(gchar** p_buf_pointer,
			     gchar** buf_pointer,
			     gint body_len,
			     GSocket* sock)
{
	gchar* p_buf = *p_buf_pointer;
	gchar* buf = *buf_pointer;

	gint pos = p_buf - buf;
	gint len = (pos / MAX_LEN + 1) * MAX_LEN;

	gchar* body = g_malloc(body_len + 1);
	memcpy(body, p_buf, MIN(len - pos, body_len));
	
	if (recv_spec_len(sock, MAX(body_len - len + pos, 0), body) == FALSE) {
		clo_socket(sock);
		g_free(body);
		return NULL;
	}
	body[body_len] = '\0';

	*p_buf_pointer = p_buf;
	*buf_pointer = buf;
	return body;
}

gboolean serv_acpt_sock(GSocket* sock,
						GIOCondition condition,
						gpointer data)
{
	if (condition == G_IO_IN) {
		gchar* buf = g_malloc(MAX_LEN);
		gchar* p_buf = buf;

		gssize nr = read_line(&p_buf, &buf, sock);
		if (nr < 0) {
			return FALSE;
		}

		gchar method[5];
		gchar* uri = g_malloc(nr);

		sscanf(p_buf, "%s%s", method, uri);

		if (strcmp(method, "GET") == 0) {
			ServGetData* data = g_new(ServGetData, 1);
			data->sock = sock;
			data->uri = uri;
			g_thread_pool_push(pool, data, NULL);

			while (nr != 2) {
				p_buf += nr;
				nr = read_line(&p_buf, &buf, sock);
				if (nr == -1) {
					return FALSE;
				}
			}
		} else

		if (strcmp(method, "POST") == 0) {
			gsize body_len = 0;

			while (nr != 2) {
				p_buf += nr;
				nr = read_line(&p_buf, &buf, sock);
				if (nr == -1) {
					return FALSE;
				}

				if (strncmp(p_buf, "Content-Length: ", MIN(nr, 14)) == 0) {
					for (int i = 0; i < nr - 18; i++) {
						body_len *= 10;
						body_len += *(p_buf + 16 + i) - '0';
					}
				}
			}

			if (body_len == 0) {
				err_400(sock);
			} else {
				gchar* body;
				p_buf += 2;
				body = read_body(&p_buf, &buf, body_len, sock);
				if (body == NULL) {
					return FALSE;
				}
				serv_post(uri, body, sock);
			}
		} else {
			err_501(sock);
		}

		GSource* source;
		source = g_main_context_find_source_by_user_data(NULL, sock);
		if (source)
			g_source_destroy(source);
		g_timeout_add_seconds(KEEP_ALIVE_TIMEOUT, (GSourceFunc)serv_acpt_sock, sock);

		g_free(buf);
		return TRUE;
	} 

	if (condition == G_IO_HUP) {
		return TRUE;
	}

	clo_socket(sock);
	return FALSE;
}

gboolean serv_listen_sock(GIOChannel* source,
						  GIOCondition condition,
						  gpointer data)
{
	GSocket* sock = (GSocket*)data;

	if (condition == G_IO_IN) {
		GSocket* acpt_sock;
		gint fd;

		acpt_sock = acpt_socket(sock);
		fd = get_socket_fd(acpt_sock);
		g_socket_set_timeout(acpt_sock, SOCK_TIMEOUT);

		GSource* source;
		source = g_socket_create_source(acpt_sock, G_IO_IN, NULL);
        g_timeout_add_seconds(KEEP_ALIVE_TIMEOUT, (GSourceFunc)serv_acpt_sock, acpt_sock);
		g_source_set_callback(source, (GSourceFunc)serv_acpt_sock, NULL, NULL);
		g_source_attach(source, NULL);
		
		return TRUE;
	}
}

void exit_func(gint sig)
{
	g_message("User requested shutdown...");
	g_thread_pool_free(pool, TRUE, FALSE);
	clo_socket(listen_sock);
	g_main_loop_quit(loop);
	g_message("EchoS is now ready to exit, bye bye...");
	exit(EXIT_SUCCESS);
}

#ifndef DEBUG
int main()
{
#ifdef G_OS_UNIX
	gint code = daemon(1, 1);
	if (code == -1) {
		g_error("%s,%d: %d %s", __FILE__, __LINE__, errno, strerror(errno));
	}
#endif
	g_type_init();
	// g_print("Welcome to use EchoS -- A very simple web server\n");
	g_message("Server started");
	signal(SIGINT, exit_func);
	signal(SIGTERM, exit_func);

	loop = g_main_loop_new(NULL, FALSE);

	gint sock_fd;
	listen_sock = ready_srv_socket(PORT);
	sock_fd = get_socket_fd(listen_sock);
	
	GIOChannel* channel;
	channel = socket_to_io_channel(sock_fd);
	g_io_add_watch(channel, G_IO_IN, serv_listen_sock, listen_sock);

	pool = g_thread_pool_new(serv_get, NULL, -1, FALSE, NULL);
	g_message("The server is now ready to accept connections\n");

	g_main_loop_run(loop);

	return 0;
}
#endif
