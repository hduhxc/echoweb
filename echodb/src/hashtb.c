#include <stdio.h>
#include <string.h>
#include "hashtb.h"
#include "murmurhash3.h"
#include "wrapper.h"

#define RECORD_NUM(block)			(*(size_b*)block)
#define RECORD_NEXT(block)			(*(size_h*)((gchar*)block + sizeof(size_b)))
#define RECORD_I_OFF(block, i)		(*(size_b*)((gchar*)block + sizeof(size_h) + (i + 1) * sizeof(size_b)))
#define P_RECORD_I(block, i)		((gchar*)block + RECORD_I_OFF(block, i))
#define RECORD_KEY_LEN(record)		(*(size_b*)record)
#define P_RECORD_KEY(record)		((gchar*)record + sizeof(size_b))
#define RECORD_VAL_LEN(record)		(*(size_b*)(P_RECORD_KEY(record) + RECORD_KEY_LEN(record)))
#define P_RECORD_VAL(record)		((gchar*)record + RECORD_KEY_LEN(record) + sizeof(size_b) * 2)
#define RECORD_LEN(record)			(RECORD_KEY_LEN(record) + RECORD_VAL_LEN(record) + sizeof(size_b) * 2)
#define P_FREE_BLOCK_BEGIN(block)	((gchar*)block + (RECORD_NUM(block) + 1) * sizeof(size_b) + sizeof(size_h))
#define P_FREE_BLOCK_END(block)		(RECORD_NUM(block) > 0 ? P_RECORD_I(block, (RECORD_NUM(block) - 1)) - 1 : block + MAX_BLOCK_SIZE - 1)
#define FREE_BLOCK_LEN(block)		(P_FREE_BLOCK_END(block) - P_FREE_BLOCK_BEGIN(block) + 1)

#define HASHTB_MASK					(*(size_h*)idx_tb)
#define HASHTB_NUM					(HASHTB_MASK + 1)
#define HASHTB_LEN					(2 * HASHTB_NUM * sizeof(size_h))
#define BLOCK_I_OFF(i)				(*((size_h*)idx_tb + i * 2 + 1))
#define BLOCK_I_MASK(i)				(*((size_h*)idx_tb + (i + 1) * 2))

gint idx_fd;
gint data_fd;
gchar* data_tb;
size_h* idx_tb;
GMappedFile* data_map_file; 

GHashTable* lock_hashtb;
GRWLock idx_lock;
GMutex mutex_lock_hashtb;

size_h cur_data_file_size;

typedef struct Data {
	GRWLock* lock;
	gchar* block;
	size_h off;
} Data;

size_h _get_file_end_off(int fd)
{
	return __filelength(fd);
}

size_h _new_block(gchar** block)
{
	size_h off = _get_file_end_off(data_fd);

	*block = (gchar*)g_malloc0(MAX_BLOCK_SIZE);
	__pwrite(data_fd, *block, MAX_BLOCK_SIZE, off);

	return off;
}

void _free_block(Data* data,
				 gboolean is_write_lock,
				 gboolean is_rewrite)
{
	if (is_write_lock == TRUE) {
		g_rw_lock_writer_unlock(data->lock);
	} else {
		g_rw_lock_reader_unlock(data->lock);
	}

	if (is_rewrite == TRUE) {
		__pwrite(data_fd, data->block, MAX_BLOCK_SIZE, data->off);
	}

	if (data->off >= cur_data_file_size) {
		g_free(data->block);
	}
	data->block = NULL;
	data->off = 0;
	data->lock = NULL;
}

static GRWLock* _get_lock(size_h off)
{
	g_assert(lock_hashtb != NULL);

	g_mutex_lock(&mutex_lock_hashtb);
	GRWLock* lock = (GRWLock*)g_hash_table_lookup(lock_hashtb, &off);

	if (lock == NULL) {
		lock = g_new(GRWLock, 1);
		g_rw_lock_init(lock);
		
		size_h* p_off = g_new(size_h, 1);
		*p_off = off;
		g_hash_table_insert(lock_hashtb, p_off, lock);
	}
	g_mutex_unlock(&mutex_lock_hashtb);

	return lock;
}

void _add_block(Data* data,
				gboolean is_write_lock)
{
	gchar* block = data->block;
	gchar* new_block;
	size_h new_off = _new_block(&new_block);

	RECORD_NEXT(block) = new_off;
	_free_block(data, is_write_lock, TRUE);

	data->block = new_block;
	data->off = new_off;
	data->lock = _get_lock(new_off);
	
	if (is_write_lock == TRUE) {
		g_rw_lock_writer_lock(data->lock);
	} else {
		g_rw_lock_reader_lock(data->lock);
	}
}

gint32 _hash_with_mask(KOrV key,
					   size_h mask)
{
	size_h hash_code;
	MurmurHash3_x86_32(key.v, key.l, 0xEE6B27EB, &hash_code);
	
	return hash_code & mask;
}

gchar* _get_from_file(gint fd, size_h off,
					  size_h len)
{
	g_assert(fd != 0);

	gssize nr;
	gchar* buf = (gchar*)g_malloc(len);
	nr = __pread(fd, buf, len, off);

	return nr < len ? NULL : buf;
}

void _get_block(Data* data,
				gboolean is_write_lock)
{
	data->lock = _get_lock(data->off);

	if (is_write_lock == TRUE) {
		g_rw_lock_writer_lock(data->lock);
	} else {
		g_rw_lock_reader_lock(data->lock);
	}

	if (data->off < cur_data_file_size) {
		data->block = data_tb + data->off;
	} else {
		data->block = _get_from_file(data_fd, data->off, MAX_BLOCK_SIZE);
	}
}

gssize _get_record_index_mem(Data* data,
							 KOrV key)
{
	gchar* block = data->block;
	g_assert(block != NULL);

	size_b num = RECORD_NUM(block);
	gssize i;

	for (i = 0; i < num; i++) {
		gchar* record = P_RECORD_I(block, i);

		if (RECORD_KEY_LEN(record) == key.l 
			&& memcmp(P_RECORD_KEY(record), key.v, key.l) == 0) {
			return i;
		}
	}

	return -1;
}

gssize _get_record_index(Data* data,
						 KOrV key,
						 gboolean is_write_lock)
{
	_get_block(data, is_write_lock);

	while (data->block != NULL) {
		gssize pos = _get_record_index_mem(data, key);

		if (pos != -1) {
			return pos;
		}

		size_h next = RECORD_NEXT(data->block);
		_free_block(data, is_write_lock, FALSE);
		
		if (next != 0) {
			data->off = next;
			_get_block(data, is_write_lock);
		}
	}

	return -1;
}

gssize _add_record_mem(Data* data,
					   KOrV key,
					   KOrV val)
{
	g_assert(data->block != NULL);

	size_b len = key.l + val.l + 2 * sizeof(size_b);

	while (FREE_BLOCK_LEN(data->block) < len + sizeof(size_b)) {
		size_h off;
		off = RECORD_NEXT(data->block);

		if (off == 0) {
			_add_block(data, TRUE);
		} else {
			_free_block(data, TRUE, FALSE);
			data->off = off;
			_get_block(data, TRUE);
		}
	}

	gchar* block = data->block;
	gchar* record = P_FREE_BLOCK_END(block) - len + 1;
	RECORD_KEY_LEN(record) = key.l;
	memcpy(P_RECORD_KEY(record), key.v, key.l);
	RECORD_VAL_LEN(record) = val.l;
	memcpy(P_RECORD_VAL(record), val.v, val.l);

	RECORD_I_OFF(block, RECORD_NUM(block)++) = record - block;

	return 0;
}

gssize _add_record(Data* data,
				   KOrV key,
				   KOrV val)
{
	_get_block(data, TRUE);
	_add_record_mem(data, key, val);

	_free_block(data, TRUE, TRUE);
	
	return 0;
}

gssize _del_record_mem(Data* data,
					   size_b index)
{
	g_assert(data->block != NULL);

	gchar* block = data->block;
	gchar* p;
	gssize i;

	p = index == 0 ? block + MAX_BLOCK_SIZE : P_RECORD_I(block, index - 1);
	size_b num = RECORD_NUM(block);

	for (i = index + 1; i < num; i++) {
		gchar* record = P_RECORD_I(block, i);
		size_b len = RECORD_LEN(record);
		
		p -= len;
		memcpy(p, P_RECORD_I(block, i), len);
		RECORD_I_OFF(block, i - 1) = p - block;
	}

	RECORD_I_OFF(block, num - 1) = 0;
	RECORD_NUM(block)--;

	return 0;
}

gssize _del_record(Data* data,
				   KOrV key)
{
	gchar* block;
	gssize index;
	
	index = _get_record_index(data, key, TRUE);

	if (index == -1) {
		return -1;
	}

	_del_record_mem(data, index);
	_free_block(data, TRUE, TRUE);

	return 0;
}

gssize _update_record(Data* data,
					  KOrV key,
					  KOrV val)
{
	gchar* block;
	gssize pos;

	pos = _get_record_index(data, key, TRUE);

	if (pos == -1) {
		return -1;
	}

	_del_record_mem(data, pos);
	_add_record_mem(data, key, val);
	
	_free_block(data, TRUE, TRUE);

	return 0;
}

void _split_table_item(Data* data_a,
					   size_h index)
{
	gssize i;

	size_h block_mask = BLOCK_I_MASK(index);
	size_h new_block_mask = (block_mask << 1) | 1;
	size_h last = index & block_mask;

	size_h block_a_last = last;
	size_h block_b_last = last | (block_mask + 1);

	Data data_b;
	data_b.off = _new_block(&data_b.block);
	data_b.lock = _get_lock(data_b.off);
	g_rw_lock_writer_lock(data_b.lock);

	if (block_mask == HASHTB_MASK) {
		size_h hashtb_len = 2 * HASHTB_LEN + sizeof(size_h);
		size_h hashtb_num = 2 * HASHTB_NUM;

		idx_tb = (size_h*)g_realloc(idx_tb, hashtb_len);
		HASHTB_MASK = (HASHTB_MASK << 1) | 1;

		for (i = hashtb_num / 2; i < hashtb_num; i++) {
			BLOCK_I_OFF(i) = BLOCK_I_OFF((i - hashtb_num / 2));
			BLOCK_I_MASK(i) = BLOCK_I_MASK((i - hashtb_num / 2));
		}
	}

	for (i = 0; i <= HASHTB_MASK; i++) {
		if ((i & new_block_mask) == block_a_last) {
			BLOCK_I_OFF(i) = data_a->off;
			BLOCK_I_MASK(i) = new_block_mask;
		}
		if ((i & new_block_mask) == block_b_last) {
			BLOCK_I_OFF(i) = data_b.off;
			BLOCK_I_MASK(i) = new_block_mask;
		}
	}

	while (data_a->block != NULL) {
		gssize i = 0;

		while (i < RECORD_NUM(data_a->block)) {
			gchar* record = P_RECORD_I(data_a->block, i);

			KOrV s_key = { RECORD_KEY_LEN(record), P_RECORD_KEY(record) };
			KOrV s_val = { RECORD_VAL_LEN(record), P_RECORD_VAL(record) };

			size_h hash_code = _hash_with_mask(s_key, new_block_mask);

			if (hash_code == block_b_last) {
				_add_record_mem(&data_b, s_key, s_val);
				_del_record_mem(data_a, i);
				continue;
			}
			i++;
		}

		size_h off = RECORD_NEXT(data_a->block);
		_free_block(data_a, TRUE, TRUE);

		if (off != 0) {
			data_a->off = off;
			_get_block(data_a, TRUE);
		}
	}

	_free_block(&data_b, TRUE, TRUE);
}

RespStat get_kv_pair_val(KOrV key,
						 KOrV* val)
{
	g_assert(idx_tb != NULL);

	size_b len;

	g_rw_lock_reader_lock(&idx_lock);
	size_h index = _hash_with_mask(key, HASHTB_MASK);
	size_h off = BLOCK_I_OFF(index);

	Data data = { NULL, NULL, off };
	gssize pos;
	pos = _get_record_index(&data, key, FALSE);

	if (pos == -1) {
		g_rw_lock_reader_unlock(&idx_lock);
		val->l = 0;
		val->v = NULL;
		return RESP_STAT_NOT_EXIST;
	}

	gchar* record = P_RECORD_I(data.block, pos);
	gchar* p_val = P_RECORD_VAL(record);
	len = RECORD_VAL_LEN(record);

	val->l = len;
	val->v = (gchar*)g_memdup(p_val, len);

	_free_block(&data, FALSE, FALSE);
	g_rw_lock_reader_unlock(&idx_lock);

	return RESP_STAT_OK;
}

RespStat add_kv_pair(KOrV key,
					 KOrV val)
{
	g_assert(idx_tb != NULL);

	size_b len = key.l + val.l + 3 * sizeof(size_b);

	g_rw_lock_writer_lock(&idx_lock);
	size_h index = _hash_with_mask(key, HASHTB_MASK);
	size_h off = BLOCK_I_OFF(index);

	gssize pos;
	Data data = { NULL, NULL, off };
	pos = _get_record_index(&data, key, TRUE);

	if (pos != -1) {
		_free_block(&data, TRUE, FALSE);
		g_rw_lock_writer_unlock(&idx_lock);
		return RESP_STAT_EXIST;
	}

	while (1) {
		index = _hash_with_mask(key, HASHTB_MASK);
		off = BLOCK_I_OFF(index);
		data = (Data){ NULL, NULL, off };
		_get_block(&data, TRUE);

		if (FREE_BLOCK_LEN(data.block) < len 
			&& HASHTB_LEN + sizeof(size_h) < MAX_HASHTB_SIZE) {

			_split_table_item(&data, index);
			continue;
		}
		_add_record_mem(&data, key, val);
		_free_block(&data, TRUE, TRUE);
		break;
	}
	g_rw_lock_writer_unlock(&idx_lock);

	return RESP_STAT_OK;
}

RespStat del_kv_pair(KOrV key)
{
	g_assert(idx_tb != NULL);

	g_rw_lock_writer_lock(&idx_lock);
	size_h index = _hash_with_mask(key, HASHTB_MASK);
	size_h off = BLOCK_I_OFF(index);

	Data data = { NULL, NULL, off };

	gssize pos;
	pos = _get_record_index(&data, key, TRUE);

	if (pos == -1) {
		g_rw_lock_writer_unlock(&idx_lock);
		return RESP_STAT_NOT_EXIST;
	}
	
	_del_record_mem(&data, pos);
	_free_block(&data, TRUE, TRUE);

	g_rw_lock_writer_unlock(&idx_lock);

	return RESP_STAT_OK;
}

RespStat update_kv_pair(KOrV key,
						KOrV val) 
{
	g_assert(idx_tb != NULL);

	g_rw_lock_writer_lock(&idx_lock);
	size_h index = _hash_with_mask(key, HASHTB_MASK);
	size_h off = BLOCK_I_OFF(index);
	
	size_b pos;
	Data data = { NULL, NULL, off };
	pos = _get_record_index(&data, key, TRUE);

	if (pos == -1) {
		g_rw_lock_writer_unlock(&idx_lock);
		return RESP_STAT_NOT_EXIST;
	}

	_del_record_mem(&data, pos);
	_add_record_mem(&data, key, val);
	_free_block(&data, TRUE, TRUE);

	g_rw_lock_writer_unlock(&idx_lock);

	return RESP_STAT_OK;
}

void _init_lock_hashtb()
{
	g_assert(lock_hashtb == NULL);

	lock_hashtb = g_hash_table_new(g_int_hash, g_int_equal);
}

void _free_lock_hashtb()
{
	g_assert(lock_hashtb != NULL);

	g_hash_table_destroy(lock_hashtb);
	lock_hashtb = NULL;
}

void init_hashtb()
{
	idx_fd = __open(IDX_FILE_NAME);
	data_fd = __open(TB_FILE_NAME);

	gsize idx_len = __filelength(idx_fd);

	if (idx_len == 0) {
		gchar* block;
		idx_len = sizeof(size_h) * 5;
		idx_tb = (size_h*)g_malloc(idx_len);

		HASHTB_MASK = 1;
		BLOCK_I_OFF(0) = _new_block(&block);
		BLOCK_I_MASK(0) = 1;
		g_free(block);
		BLOCK_I_OFF(1) = _new_block(&block);
		BLOCK_I_MASK(1) = 1;
		g_free(block);
	} else {
		idx_tb = (size_h*)_get_from_file(idx_fd, 0, idx_len);
	}

	data_map_file = g_mapped_file_new_from_fd(data_fd, TRUE, NULL);
	data_tb = g_mapped_file_get_contents(data_map_file);
	cur_data_file_size = __filelength(data_fd);
	
	_init_lock_hashtb();

	g_message("DB loaded from disk");
}

void free_hashtb()
{
	g_mapped_file_unref(data_map_file);

	size_h len = HASHTB_LEN + sizeof(size_h);
	__pwrite(idx_fd, idx_tb, len, 0);
	g_free(idx_tb);
	idx_tb = NULL;

	__close(idx_fd);
	__close(data_fd);

	g_rw_lock_clear(&idx_lock);
	_free_lock_hashtb();

	g_message("DB saved on disk");
}
