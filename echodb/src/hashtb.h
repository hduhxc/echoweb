#ifndef _HASHTB_H
#define _HASHTB_H

#include "glib.h"
#include "protocol.h"
#define MAX_HASHTB_SIZE (8 * 1024 * 1024 + 4)
#define MAX_BLOCK_SIZE (8 * 1024)
#define IDX_FILE_NAME "data/idx_file.edb"
#define TB_FILE_NAME "data/data_file.edb"

typedef guint16 size_b;
typedef guint32 size_h;
typedef gboolean gbool;

typedef struct KOrV {
	size_b l;
	gchar* v;
} KOrV;

RespStat get_kv_pair_val(KOrV key, KOrV* val);
RespStat add_kv_pair(KOrV key, KOrV val);
RespStat del_kv_pair(KOrV key);
RespStat update_kv_pair(KOrV key, KOrV val);
void init_hashtb();
void free_hashtb();

#endif
