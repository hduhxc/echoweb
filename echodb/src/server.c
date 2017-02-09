#include <stdlib.h>
#include <signal.h>
#include "inet.h"
#include "protocol.h"
#include "hashtb.h"
#include "wrapper.h"

#ifdef G_OS_UNIX
#include <string.h>
#include <errno.h>
#include <unistd.h>
#endif

#define PORT 6666
#define START_SVR

GMainLoop* loop;
GThreadPool* pool;
GCond cond;
GMutex mutex;

typedef struct ReqData {
	GSocket* sock;
	gint ret;
} ReqData;

void serv_req(gpointer data,
			  gpointer user_data)
{
	GSocket* sock = ((ReqData*)data)->sock;
	len_t recv_len;
	gchar* recv_buf;
	Req req;
	Resp resp;

	recv_buf = (gchar*)g_new(len_t, 1);
	recv_spec_len(sock, sizeof(len_t), recv_buf);

	recv_len = *(len_t*)recv_buf;
	if (recv_len == 0) {
		clo_socket(sock);
		((ReqData*)data)->ret = -1;
		g_cond_signal(&cond);
		return;
	}

	recv_buf = (gchar*)g_realloc(recv_buf, recv_len);
	recv_spec_len(sock, recv_len - sizeof(len_t), recv_buf + sizeof(len_t));
	g_cond_signal(&cond);

	parse_req(recv_buf, &req);
	
	KOrV key = { req.key_len, req.key };
	KOrV val = { req.val_len, req.val };

	switch (req.meth) {
		case REQ_METH_I: {
				resp.stat = add_kv_pair(key, val);
				break;
			}
		case REQ_METH_G: {
				resp.stat = get_kv_pair_val(key, &val);
				resp.val_len = val.l;
				resp.val = val.v;
				break;
			}
		case REQ_METH_U: {
				resp.stat = update_kv_pair(key, val);
				break;
			}
		case REQ_METH_D: {
				resp.stat = del_kv_pair(key);
				break;
			}
	}
	resp.meth = req.meth;

	len_t send_len;
	gchar* send_buf;

	send_len = gen_resp(&resp, &send_buf);
	send_spec_len(sock, send_len, send_buf);

	g_free(recv_buf);
	g_free(send_buf);
}

gboolean serv_acpt_sock(GIOChannel* source,
						GIOCondition condition,
						gpointer data)
{
	GSocket* sock = (GSocket*)data;
	ReqData req_data = { sock, 0 };

	if (condition == G_IO_IN) {
		g_thread_pool_push(pool, &req_data, NULL);
		g_mutex_lock(&mutex);
		g_cond_wait(&cond, &mutex);
		g_mutex_unlock(&mutex);

		if (req_data.ret == -1) {
			return FALSE;
		} else {
			return TRUE;
		}
	}

	if (condition == G_IO_HUP) {
		clo_socket(sock);

		return FALSE;
	}
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
		g_socket_set_timeout(acpt_sock, 60);

		GIOChannel* channel;
		channel = socket_to_io_channel(fd);
		g_io_add_watch(channel, G_IO_IN, serv_acpt_sock, acpt_sock);

		return TRUE;
	}
}

void exit_func(gint sig)
{
	g_message("User requested shutdown...");
	g_thread_pool_free(pool, TRUE, FALSE);
	free_hashtb();
	g_message("EchoDB is now ready to exit, bye bye...");
	g_main_loop_quit(loop);
	exit(EXIT_SUCCESS);
}

#ifdef START_SVR

int main()
{
#ifdef G_OS_UNIX
	gint code = daemon(1, 1);
	if (code == -1) {
		g_error("%s,%d: %d %s", __FILE__, __LINE__, errno, strerror(errno));
	}
#endif
	g_type_init();
	// g_print("Welcome to use EchoDB -- A very simple Key-Value database\n");
	g_message("Server started");
	signal(SIGINT, exit_func);
	signal(SIGTERM, exit_func);

	init_hashtb();
	loop = g_main_loop_new(NULL, FALSE);

	GSocket* sock;
	gint sock_fd;
	sock = ready_srv_socket(PORT);
	sock_fd = get_socket_fd(sock);

	GIOChannel* channel;
	channel = socket_to_io_channel(sock_fd);
	g_io_add_watch(channel, G_IO_IN, serv_listen_sock, sock);
	
	pool = g_thread_pool_new(serv_req, NULL, -1, FALSE, NULL);
	g_message("The server is now ready to accept connections\n");

	g_main_loop_run(loop);

	return 0;
}

#endif
