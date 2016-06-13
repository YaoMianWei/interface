

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include"cJSON.h"


#define return_val_if_fail(p,ret) if(!(p))\
{printf("%s:%d warning "#p" failed.\n",\
		__func__, __LINE__); return (ret);}

#define MALLOCSIZE 1024


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

uint32_t GSNO;

typedef enum _ParseRet
{
	PARSE_RET_OK,
	PARSE_RET_OOM,
	PARSE_RET_STOP,
	PARSE_RET_PARAMS,
	PARSE_RET_FAIL
}ParseRet;

typedef struct _binmessage
{
	uint8_t *binMess;
	uint32_t binLen;
	uint8_t *devIds;
	uint32_t devLen;
	uint32_t newSno;
	uint32_t oldSno;
	uint32_t type;
}BinMessage;

typedef struct _memoryManage
{
	uint8_t *data;
	uint32_t mmSize;
	uint32_t dataLen;
}MmManage;

typedef struct _paramContent
{
	uint32_t partSum;
	uint32_t itemSum;
	uint32_t partPos;
	uint32_t itemPos;
	uint32_t partX;
	uint32_t partY;
	uint32_t partWidth;
	uint32_t partHeight;
	uint32_t partGifFlag;
	uint32_t partBorderWidth;
	uint32_t partColorType;
	uint32_t countPage;
	uint32_t proId;
	
}ParamContent;


typedef struct _recMessage
{
	uint8_t *json;
	uint8_t *topic;
}RecMessage;

typedef struct _anmi
{
	uint8_t type;
	uint8_t speed;
	uint8_t time;
}Anmi;

typedef BinMessage* (JsonParseFunc)(cJSON* cJsonRoot);

//单包指令组包
//BinMessage* sim_join_package(cJSON* cjsonRoot, JsonParseFunc parse);

//多包节目组包
//BinMessage* mul_join_package(cJSON* cjsonRoot);

//json转bin接口
BinMessage * interface_json_to_bin(uint8_t* jsonString);

void buf_print(uint8_t* buf, int length);

void bm_print(BinMessage *bm);

#endif
