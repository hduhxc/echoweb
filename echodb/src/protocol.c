#include <string.h>
#include "protocol.h"

#define REQ_LEN(req)		(*(len_t*)req)
#define REQ_METHOD(req)		(*((gchar*)req + sizeof(len_t)))
#define REQ_KEY_LEN(req)	(*(len_t*)((gchar*)req + sizeof(len_t) + 1))
#define REQ_KEY(req)		((gchar*)req + 1 + 2 * sizeof(len_t))
#define REQ_VAL_LEN(req)	(*(len_t*)(REQ_KEY(req) + REQ_KEY_LEN(req)))
#define REQ_VAL(req)		(REQ_KEY(req) + REQ_KEY_LEN(req) + sizeof(len_t))

#define RESP_LEN(resp)		(*(len_t*)resp)
#define RESP_METH(resp)		(*((gchar*)resp + sizeof(len_t)))
#define RESP_STAT(resp)		(*(stat_t*)((gchar*)resp + 1 + sizeof(len_t)))
#define RESP_VAL_LEN(resp)  (*(len_t*)((gchar*)resp + 1 + sizeof(len_t) + sizeof(stat_t)))
#define RESP_VAL(resp)		((gchar*)resp + 1 + sizeof(len_t) * 2 + sizeof(stat_t))

len_t gen_req(Req* req,
			  gchar** p_buf)
{
	gchar* buf = NULL;
	len_t buf_len = 0;

	if (req->meth == REQ_METH_G) {
		buf_len = sizeof(len_t) * 2 + req->key_len + 1;
		buf = (gchar*)g_malloc(buf_len);

		REQ_METHOD(buf) = 'G';
		REQ_KEY_LEN(buf) = req->key_len;
		memcpy(REQ_KEY(buf), req->key, req->key_len);
	}

	if (req->meth == REQ_METH_I) {
		buf_len = sizeof(len_t) * 3 + req->key_len + req->val_len + 1;
		buf = (gchar*)g_malloc(buf_len);

		REQ_METHOD(buf) = 'I';
		REQ_KEY_LEN(buf) = req->key_len;
		memcpy(REQ_KEY(buf), req->key, req->key_len);
		REQ_VAL_LEN(buf) = req->val_len;
		memcpy(REQ_VAL(buf), req->val, req->val_len);
	}

	if (req->meth == REQ_METH_U) {
		buf_len = sizeof(len_t) * 3 + req->key_len + req->val_len + 1;
		buf = (gchar*)g_malloc(buf_len);

		REQ_METHOD(buf) = 'U';
		REQ_KEY_LEN(buf) = req->key_len;
		memcpy(REQ_KEY(buf), req->key, req->key_len);
		REQ_VAL_LEN(buf) = req->val_len;
		memcpy(REQ_VAL(buf), req->val, req->val_len);
	}

	if (req->meth == REQ_METH_D) {
		buf_len = sizeof(len_t) * 2 + req->key_len + 1;
		buf = (gchar*)g_malloc(buf_len);

		REQ_METHOD(buf) = 'D';
		REQ_KEY_LEN(buf) = req->key_len;
		memcpy(REQ_KEY(buf), req->key, req->key_len);
	}
	REQ_LEN(buf) = buf_len;

	*p_buf = buf;
	return buf_len;
}

gssize parse_req(gchar* buf,
				 Req *req)
{
	gchar meth_ch = REQ_METHOD(buf);

	if (meth_ch == 'G') {
		req->meth = REQ_METH_G;
		req->key_len = REQ_KEY_LEN(buf);
		req->key = REQ_KEY(buf);
	}

	if (meth_ch == 'D') {
		req->meth = REQ_METH_D;
		req->key_len = REQ_KEY_LEN(buf);
		req->key = REQ_KEY(buf);
	}

	if (meth_ch == 'I') {
		req->meth = REQ_METH_I;
		req->key_len = REQ_KEY_LEN(buf);
		req->key = REQ_KEY(buf);
		req->val_len = REQ_VAL_LEN(buf);
		req->val = REQ_VAL(buf);
	}

	if (meth_ch == 'U') {
		req->meth = REQ_METH_U;
		req->key_len = REQ_KEY_LEN(buf);
		req->key = REQ_KEY(buf);
		req->val_len = REQ_VAL_LEN(buf);
		req->val = REQ_VAL(buf);
	}

	return 0;
}

len_t gen_resp(Resp* resp,
		       gchar** p_buf)
{
	len_t buf_len;
	gchar* buf;

	buf_len = sizeof(len_t) + 1 + sizeof(stat_t);
	buf = (gchar*)g_malloc(buf_len);

	switch (resp->meth) {
		case REQ_METH_U: {
				RESP_METH(buf) = 'U';
				break;
			}
		case REQ_METH_D: {
				RESP_METH(buf) = 'D';
				break;
			}
		case REQ_METH_I: {
				RESP_METH(buf) = 'I';
				break;
			}
		case REQ_METH_G: {
				RESP_METH(buf) = 'G';

				if (resp->meth == RESP_STAT_OK) {
					buf_len += sizeof(len_t) + resp->val_len;
					buf = (gchar*)g_realloc(buf, buf_len);

					RESP_VAL_LEN(buf) = resp->val_len;
					memcpy(RESP_VAL(buf), resp->val, resp->val_len);
				}
				break;
			}
	}

	RESP_LEN(buf) = buf_len;
	RESP_STAT(buf) = resp->stat;
	
	*p_buf = buf;
	return buf_len;
}

gssize parse_resp(gchar* buf,
				  Resp* resp)
{
	resp->stat = (RespStat)RESP_STAT(buf);

	switch (RESP_METH(buf)) {
		case 'I': {
				resp->meth = REQ_METH_I;
				break;
			}
		case 'D': {
				resp->meth = REQ_METH_D;
				break;
			}
		case 'U': {
				resp->meth = REQ_METH_U;
				break;
			}
		case 'G': {
				resp->meth = REQ_METH_G;

				if (resp->stat == RESP_STAT_OK) {
					resp->val_len = RESP_VAL_LEN(buf);
					resp->val = (gchar*)g_memdup(RESP_VAL(buf), resp->val_len);
				} else {
					resp->val_len = 0;
					resp->val = NULL;
				}
				break;
			}
	}

	return 0;
}
