#ifndef _CLIENT_H
#define _CLIENT_H

#ifdef _WIN32

#ifdef DLL_EXPORT
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#else
#define EXPORT 
#endif

typedef enum RespStat {
	RESP_STAT_OK,
	RESP_STAT_EXIST,
	RESP_STAT_NOT_EXIST,
	RESP_STAT_UNKNOWN
} RespStat;

EXPORT RespStat echodb_get(unsigned int key_len, char* key, unsigned int* val_len, char** val);
EXPORT RespStat echodb_set(unsigned int key_len, char* key, unsigned int val_len, char* val);
EXPORT RespStat echodb_update(unsigned int key_len, char* key, unsigned int val_len, char* val);
EXPORT RespStat echodb_delete(unsigned int key_len, char* key);
EXPORT void echodb_connect(char* ip_addr, unsigned short port);
EXPORT void echodb_close();

#endif
