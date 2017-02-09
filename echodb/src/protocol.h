#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "glib.h"
#include "client.h"

typedef guint16 len_t;
typedef guint8 stat_t;
typedef enum ReqMeth {
	REQ_METH_G,
	REQ_METH_U,
	REQ_METH_I,
	REQ_METH_D
} ReqMeth;

typedef struct Req {
	ReqMeth meth;
	len_t key_len;
	gchar* key;
	len_t val_len;
	gchar* val;
} Req;

typedef struct Resp {
	RespStat stat;
	ReqMeth meth;
	len_t val_len;
	gchar* val;
} Resp;

len_t gen_req(Req* req, gchar** p_buf);
gssize parse_req(gchar* buf, Req *req);
len_t gen_resp(Resp* resp, gchar** p_buf);
gssize parse_resp(gchar* buf, Resp* resp);

#endif