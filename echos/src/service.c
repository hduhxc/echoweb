#include <stdio.h>
#include <string.h>
#include "service.h"
#include "inet.h"
#include "config.h"
#include "my_lib.h"

#define _handle_error(err)	do { \
								if (err != NULL) { \
									g_error("%s,%d: %d %s", __FILE__, __LINE__, err->code, err->message); \
								} \
							} while(0)

typedef struct URI {
	gchar* MIME;
	gchar* filename;
	gchar* query;

} URI;

typedef enum FileStat {
	FILE_NOT_EXISTS,
	FILE_EXISTS,
	FILE_EXECUTABLE
} FileStat;

void err_404(GSocket* sock)
{
	gchar buf[MAX_HEADER_LEN];

	sprintf(buf,
			"HTTP/1.1 404 NOT FOUND\r\n"
			"Server: %s\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 157\r\n"
			"\r\n"
			"<html><body>"
			"<h3>404 Not Found</h3>"
			"<p>The server could not fulfill your request because the resource specified is unavailable or nonexistent</p>"
			"</body></html>",
			SERVER_NAME);
	
	send_spec_len(sock, strlen(buf), buf);
}

void err_501(GSocket* sock)
{
	gchar buf[MAX_HEADER_LEN];

	sprintf(buf,
			"HTTP/1.1 501 Method Not Implemented\r\n"
			"Server: %s\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 101\r\n"
			"\r\n"
			"<html><body>"
			"<h3>501 Method Not Implemented</h3>"
			"<p>HTTP request method not supported</p>"
			"</body></html>",
			SERVER_NAME);

	send_spec_len(sock, strlen(buf), buf);
}

void err_500(GSocket* sock)
{
	gchar buf[MAX_HEADER_LEN];

	sprintf(buf,
			"HTTP/1.1 500 Internal Server Error\r\n"
			"Content-Type: text/html\r\n"
			"Server: %s\r\n"
			"Content-Length: 97\r\n"
			"\r\n"
			"<html><body>"
			"<h3>500 Internal Server Error</h3>"
			"<p>Error prohibited CGI execution</p>"
			"</body></html>",
			SERVER_NAME);

	send_spec_len(sock, strlen(buf), buf);
}

void err_400(GSocket* sock)
{
	gchar buf[MAX_HEADER_LEN];

	sprintf(buf,
			"HTTP/1.1 400 Bad Request\r\n"
			"Content-Type: text/html\r\n"
			"Server: %s\r\n"
			"Content-Length: 129\r\n"
			"\r\n"
			"<html><body>"
			"<h3>400 Bad Request</h3>"
			"<p>Your browser sent a bad request, such as a POST without a Content-Length</p>"
			"</body></html>",
			SERVER_NAME);

	send_spec_len(sock, strlen(buf), buf);
}

void err_200(GSocket* sock,
			 gsize len,
			 gchar* type)
{
	gchar buf[MAX_HEADER_LEN];
	gchar tmp[MAX_HEADER_LEN];

	if (len != 0) {
		sprintf(tmp, "Content-Length: %d\r\n", len);
	} else {
		sprintf(tmp, "Transfer-Encoding: chunked\r\n");
	}

	sprintf(buf,
			"HTTP/1.1 200 OK\r\n"
			"Server: %s\r\n"
			"Content-Type: %s\r\n"
			"%s"
			"\r\n",
			SERVER_NAME, type, tmp);

	send_spec_len(sock, strlen(buf), buf);
}

FileStat _test_file(const gchar* filename)
{
	if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
		return FILE_NOT_EXISTS;
	}
	if (g_file_test(filename, G_FILE_TEST_IS_EXECUTABLE)
	|| strstr(filename, ".el")) {

		return FILE_EXECUTABLE;
	} else {
		return FILE_EXISTS;
	}
}

void _free_uri(URI* uri)
{
	g_free(uri->filename);
	g_free(uri->query);
}

URI _parse_uri(gchar* uri)
{
	URI s_uri;
	gchar* split_pos;
	gchar* filename;
	gchar* query;
	split_pos = strchr(uri, '?');

	if (split_pos == NULL) {
		split_pos = uri + strlen(uri);
		s_uri.query = NULL;
	} else {
		s_uri.query = g_strdup(split_pos);
	}

	gsize dir_len = strlen(WWW_DIR);
	gsize file_len = split_pos - uri;
	gsize len = dir_len + file_len + 1;

	filename = g_malloc(len + strlen(DEFAULT_PAGE));
	memcpy(filename + dir_len, uri, file_len);
	memcpy(filename, WWW_DIR, dir_len);

	filename[len - 1] = '\0';
	if (*(split_pos - 1) == '/') {
		strcat(filename, DEFAULT_PAGE);
	}
	s_uri.filename = filename;
	s_uri.MIME = NULL;

	if (strstr(filename, ".html")) {
		s_uri.MIME = "text/html";
	}
	if (strstr(filename, ".el")) {
		s_uri.MIME = "application/x-echol";
	}

	return s_uri;
}

void _free_map_file(GMappedFile* map_file)
{
	g_mapped_file_unref(map_file);
}

GMappedFile* _get_map_file(gchar* filename)
{
	GError* err = NULL;
	gsize file_len;
	GMappedFile* map_file = g_mapped_file_new(filename, FALSE, &err);
	_handle_error(err);

	return map_file;
}

gsize _get_file(GMappedFile* map_file,
			    gchar** file)
{
	gsize len;
	*file = g_mapped_file_get_contents(map_file);
	len = g_mapped_file_get_length(map_file);

	return len;
}

void serv_executable(URI* uri,
					 gchar* body,
					 GSocket* sock)
{
	gchar* env[3] = { NULL };
	gchar* argv[3] = { NULL };

	if (g_strcmp0(uri->MIME, "application/x-echol") == 0) {
		argv[0] = ECHOL_INTERPRETER;
		argv[1] = uri->filename;
	} else {
		argv[0] = uri->filename;
	}

	gsize count = 0;
	gsize body_len = 0;

	if (body != NULL) {
		gchar str_len[10];
		body_len = strlen(body);
		sprintf(str_len, "%d", body_len);
		env[count++] = strappend("CONTENT_LENGTH=", str_len);
	}
	if (uri->query != NULL) {
		env[count++] = strappend("QUERY_STRING", uri->query);
	}

	gint input;
	gint output;

	GError* err = NULL;
	g_spawn_async_with_pipes(NULL, argv, env,
							 0,
							 NULL, NULL,
							 NULL,
							 &input, &output, NULL,
							 &err);
	_handle_error(err);

	if (body_len != 0) {
		fd_send_spec_len(input, body, body_len);
	}
	Close(input);
	err_200(sock, 0, "text/html");

	gchar buf[MAX_LEN + 12];
	gchar len[11];

	while (TRUE) {
		gchar* p_buf;
		gint nr;
		gint len_size;
		gboolean stat;
		
		p_buf = buf + 10;
		nr = Read(output, p_buf, MAX_LEN);
		p_buf[nr] = '\r';
		p_buf[nr + 1] = '\n';

		sprintf(len, "%x\r\n", nr);
		len_size = strlen(len);
		p_buf -= len_size;
		memcpy(p_buf, len, len_size);

		stat = send_spec_len(sock, len_size + nr + 2, p_buf);
		if (stat == FALSE) {
			break;
		}

		if (nr == 0) {
			Close(output);
			break;
		}
	}
}

void serv_post(gchar* uri_str,
			   gchar* body,
			   GSocket* sock)
{
	URI uri = _parse_uri(uri_str);
	FileStat stat = _test_file(uri.filename);

	if (stat != FILE_EXECUTABLE) {
		err_500(sock);
		return;
	}

	serv_executable(&uri, body, sock);

	_free_uri(&uri);
	g_free(body);
}

void _free_serv_get_data(ServGetData* data)
{
	g_free(data->uri);
	g_free(data);
}

void serv_get(gpointer data,
			  gpointer user_data)
{
	ServGetData* serv_get_data;
	URI uri;
	FileStat stat;

	serv_get_data = (ServGetData*)data;
	uri  = _parse_uri(serv_get_data->uri);
	stat = _test_file(uri.filename);

	if (stat == FILE_EXECUTABLE) {
		serv_executable(&uri, NULL, serv_get_data->sock);
		_free_serv_get_data(serv_get_data);
		_free_uri(&uri);
		return;
	}
	if (stat == FILE_NOT_EXISTS) {
		err_404(serv_get_data->sock);
		_free_serv_get_data(serv_get_data);
		_free_uri(&uri);
		return;
	}
	
	GMappedFile* map_file;
	gchar* file;
	gsize len;

	map_file = _get_map_file(uri.filename);
	len = _get_file(map_file, &file);

	err_200(serv_get_data->sock, len, uri.MIME);
	send_spec_len(serv_get_data->sock, len, file);

	_free_serv_get_data(serv_get_data);
	_free_uri(&uri);
	_free_map_file(map_file);
}
