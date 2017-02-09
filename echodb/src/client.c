#include "client.h"
#include "protocol.h"
#include "inet.h"

GSocket* sock;

void _send_and_recv(Req* req, 
					Resp* resp)
{
	gchar* buf;
	len_t send_len = gen_req(req, &buf);
	send_spec_len(sock, send_len, buf);
	g_free(buf);

	buf = (gchar*)g_new(len_t, 1);
	recv_spec_len(sock, sizeof(len_t), buf);

	len_t recv_len = *(len_t*)buf;
	buf = (gchar*)g_realloc(buf, recv_len);
	recv_spec_len(sock, recv_len - sizeof(len_t), buf + sizeof(len_t));

	parse_resp(buf, resp);
	g_free(buf);
}

RespStat echodb_get(unsigned int key_len, char* key,
					unsigned int* val_len, char** val)
{
	g_assert(sock != NULL);

	Req req = { REQ_METH_G, key_len, key, 0, NULL };
	Resp resp;

	_send_and_recv(&req, &resp);

	*val_len = resp.val_len;
	*val = resp.val;

	return resp.stat;
}

RespStat echodb_set(unsigned int key_len, char* key,
					unsigned int val_len, char* val)
{
	g_assert(sock != NULL);

	Req req = { REQ_METH_I, key_len, key, val_len, val };
	Resp resp;

	_send_and_recv(&req, &resp);

	return resp.stat;
}

RespStat echodb_update(unsigned int key_len, char* key,
					   unsigned int val_len, char* val)
{
	g_assert(sock != NULL);

	Req req = { REQ_METH_U, key_len, key, val_len, val };
	Resp resp;

	_send_and_recv(&req, &resp);

	return resp.stat;
}

RespStat echodb_delete(unsigned int key_len, char* key)
{
	g_assert(sock != NULL);

	Req req = { REQ_METH_D, key_len, key, 0, NULL };
	Resp resp;

	_send_and_recv(&req, &resp);

	return resp.stat;
}

void echodb_connect(char* ip_addr,
					unsigned short port)
{
	sock = ready_cli_socket(ip_addr, port);
	g_socket_set_timeout(sock, 60);
	g_socket_set_blocking(sock, TRUE);
}

void echodb_close()
{
	clo_socket(sock);
}

#ifdef _DEBUG

int main()
{
	echodb_connect("127.0.0.1", 6666);

	RespStat resp;
	unsigned int val_len;
	char* val;

	resp = echodb_set(5, "Hello", 5, "World");
	g_print("%d", resp);
	resp = echodb_get(5, "Hello", &val_len, &val);
	g_print("%d", resp);
	g_free(val);

	resp = echodb_update(5, "Hello", 5, "Japan");
	g_print("%d", resp);
	resp = echodb_get(5, "Hello", &val_len, &val);
	g_print("%d", resp);
	g_free(val);

	resp = echodb_delete(5, "Hello");
	g_print("%d", resp);
	resp = echodb_get(5, "Hello", &val_len, &val);
	g_print("%d", resp);

	return 0;
}

#endif
