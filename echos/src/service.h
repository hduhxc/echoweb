#ifndef _SERVICE_H
#define _SERVICE_H

#include "glib.h"
#include "gio/gio.h"

typedef struct ServGetData {
	GSocket* sock;
	gchar* uri;
} ServGetData;

void err_404(GSocket* sock);
void err_501(GSocket* sock);
void err_500(GSocket* sock);
void err_400(GSocket* sock);
void err_200(GSocket* sock, gsize len, gchar* type);
gboolean dispatch_func(gpointer user_data);
void serv_post(gchar* uri_str, gchar* body, GSocket* sock);
void serv_get(gpointer data, gpointer user_data);

#endif