
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"interface.h"
#include"sim.h"
#include"memory.h"


//组合单包指令

static BinMessage* sim_join_package(cJSON* cjsonRoot, JsonParseFunc parse)
{
	return_val_if_fail(cjsonRoot, NULL);

	BinMessage *bmRet = parse(cjsonRoot);

	return_val_if_fail(bmRet, NULL);

	uint32_t dataLen = bmRet->binLen;

	bmRet->binLen = 0;

	uint8_t *data[dataLen];

	memset(data, 0, dataLen);

	memcpy(data, bmRet->binMess, dataLen);
	
	uint32_t devLen = bmRet->devLen;
	
	bmRet->binMess[bmRet->binLen] = devLen & 0xff;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = devLen >> 8;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = devLen >> 16;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = devLen >> 24;
	
	bmRet->binLen ++;

	memcpy(bmRet->binMess + bmRet->binLen, bmRet->devIds, bmRet->devLen);
	
	bmRet->binLen += bmRet->devLen;
	
	bmRet->binMess[bmRet->binLen] = (dataLen+14) & 0xff;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = (dataLen+14) >> 8;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = (dataLen+14) >> 16;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = (dataLen+14) >> 24;
	
	bmRet->binLen ++;

	uint32_t headPos = bmRet->binLen; 

	bmRet->binMess[bmRet->binLen] = 0xa5;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = 0x01;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = 0x7b;
	
	bmRet->binLen ++;

	uint32_t lenPos = bmRet->binLen;

	bmRet->binMess[bmRet->binLen] = 0x00;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = 0x00;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = GSNO & 0xff;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = GSNO >> 8;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = GSNO >> 16;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = GSNO >> 24;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = 0x00;
	
	bmRet->binLen ++;
	
	bmRet->binMess[bmRet->binLen] = 0x00;
	
	bmRet->binLen ++;

	memcpy(bmRet->binMess + bmRet->binLen, data, dataLen);
	
	bmRet->binLen += dataLen;

	bmRet->binMess[lenPos] = (bmRet->binLen - headPos + 3) & 0xff;
	
	bmRet->binMess[lenPos + 1] = (bmRet->binLen - headPos + 3) >> 8;
	
	uint32_t pos = 0, checkSum = 0;
	
	for(pos = headPos; pos < bmRet->binLen; pos++)
	{		
		checkSum += bmRet->binMess[pos];
	}

	bmRet->binMess[bmRet->binLen] = checkSum & 0xff;

	bmRet->binLen++;

	bmRet->binMess[bmRet->binLen] = checkSum >> 8;
	
	bmRet->binLen++;

	bmRet->binMess[bmRet->binLen] = 0xae;

	bmRet->binLen++;	
	
	return bmRet;
}


//组合多包指令
#if 0
static BinMessage* mul_join_package(cJSON* cjson)
{
	return_val_if_fail(cjson, NULL);

	cJSON* pktsProgram = cJSON_GetObjectItem(cjson, "pkts_program");

	return_val_if_fail(pktsProgram, NULL);

	cJSON *cjsonSno = cJSON_GetObjectItem(cjson, "sno");

	return_val_if_fail(cjsonSno, NULL);

	uint32_t sno = cjsonSno->valueint;

	cJSON *cjsonDevIds = cJSON_GetObjectItem(cjson, "ids_dev");

	return_val_if_fail(cjsonDevIds, NULL);

	uint8_t* idsDev = cjsonDevIds->valuestring; 

	uint8_t proBuf[MALLOCSIZE] = {0};

	uint32_t proBufLen = get_property(pktsProgram, proBuf);
	if(proBufLen <= 0)
	{
		return NULL;
	}

	uint32_t proLen = add_head_tail(proBuf, proBufLen);

	buf_print(proBuf, proLen);

	uint32_t contentLen = 0;

	uint8_t *content = get_content(pktsProgram, &contentLen);
	if(!content)
	{
		return NULL;
	}

	BinMessage *binRet = bin_message_new();

	return_val_if_fail(binRet, NULL);

	uint32_t idsDevLen = strlen(idsDev);

	binRet->devIds = (uint8_t*)malloc(sizeof(uint8_t)*idsDevLen);
	if(!binRet->devIds)
	{
		bin_message_destroy(binRet);

		return NULL;
	}

	memcpy(binRet->devIds + binRet->devLen, idsDev, idsDevLen);

	binRet->devLen += idsDevLen;

	uint32_t mallocLen = contentLen + proLen + 8 + idsDevLen;

	uint32_t sumLen = contentLen + proLen;

	binRet->binMess = (uint8_t*)malloc(sizeof(uint8_t)*mallocLen);
	if(!binRet->binMess)
	{
		bin_message_destroy(binRet);

		return NULL;
	}

	binRet->binMess[binRet->binLen] = (idsDevLen) & 0xff;

	binRet->binLen++;

	binRet->binMess[binRet->binLen] = (idsDevLen) >> 8;

	binRet->binLen++;

	binRet->binMess[binRet->binLen] = (idsDevLen) >>16;

	binRet->binLen++;

	binRet->binMess[binRet->binLen] = (idsDevLen) >> 24;

	binRet->binLen++;

	memcpy(binRet->binMess + binRet->binLen, idsDev, idsDevLen);

	binRet->binLen += idsDevLen;

	binRet->binMess[binRet->binLen] = (sumLen) & 0xff;

	binRet->binLen++;

	binRet->binMess[binRet->binLen] = (sumLen) >> 8;

	binRet->binLen++;

	binRet->binMess[binRet->binLen] = (sumLen) >> 16;

	binRet->binLen++;

	binRet->binMess[binRet->binLen] = (sumLen) >> 24;

	binRet->binLen++;

	memcpy(binRet->binMess + binRet->binLen, proBuf, proLen);

	binRet->binLen += proLen;

	memcpy(binRet->binMess + binRet->binLen, content, contentLen);

	binRet->binLen += contentLen;

	free(content);

	return binRet;
}

#endif

//json解析接口
BinMessage * interface_json_to_bin(uint8_t* jsonString)
{
	cJSON *cmdPsub;

	BinMessage *binRet = NULL;
	
	cJSON *root = cJSON_Parse(jsonString);

	return_val_if_fail(root, NULL);

	cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
	if(cmd)
	{
		cmdPsub = cJSON_GetObjectItem(cmd, "param_dev");
		if(cmdPsub)
		{
			binRet = sim_join_package(root, screen_cfg_set);

			cJSON_Delete(root);

			return binRet;
		}

		cmdPsub = cJSON_GetObjectItem(cmd, "power");
		if(cmdPsub)
		{
			binRet = sim_join_package(root, switch_cfg_set);

			cJSON_Delete(root);

			return binRet;
		}

		cmdPsub = cJSON_GetObjectItem(cmd, "ligth");
		if(cmdPsub)
		{
			binRet = sim_join_package(root, light_cfg_set);

			cJSON_Delete(root);

			return binRet;
		}

		cmdPsub = cJSON_GetObjectItem(cmd, "param_net");
		if(cmdPsub)
		{
			binRet = sim_join_package(root, net_cfg_set);

			cJSON_Delete(root);

			return binRet;
		}

		cmdPsub = cJSON_GetObjectItem(cmd, "param_gprs");
		if(cmdPsub)
		{
			binRet = sim_join_package(root, gprs_cfg_set);

			cJSON_Delete(root);

			return binRet;
		}

		cmdPsub = cJSON_GetObjectItem(cmd, "get");
		if(cmdPsub)
		{
			binRet = sim_join_package(root, param_cfg_get);

			cJSON_Delete(root);

			return binRet;
		}
	}

#if 0

	cJSON *pktsProgram = cJSON_GetObjectItem(root, "pkts_program");
	if(pktsProgram)
	{
		binRet = mul_join_package(root);

		cJSON_Delete(root);

		return binRet;
	}

#endif

	cJSON_Delete(root);
	
	return NULL;	
}

int main(int argc, char** argv)
{
	return 0;
}
