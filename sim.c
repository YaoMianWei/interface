#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"cJSON.h"
#include"interface.h"
#include"memory.h"
#include"utils.h"

//屏幕参数的设置
BinMessage* screen_cfg_set(cJSON *cjsonRoot)
{	
	return_val_if_fail(cjsonRoot, NULL);

	cJSON *screen_cmd = cJSON_GetObjectItem(cjsonRoot, "cmd");

	return_val_if_fail(screen_cmd, NULL);

	cJSON *screen_param_dev = cJSON_GetObjectItem(screen_cmd,"param_dev");

	return_val_if_fail(screen_param_dev, NULL);

	cJSON *screen_pSub = cJSON_GetObjectItem(screen_param_dev,"width");

	return_val_if_fail(screen_pSub, NULL);

	uint16_t width = screen_pSub->valueint / 8;

	screen_pSub = cJSON_GetObjectItem(screen_param_dev,"height");

	return_val_if_fail(screen_pSub, NULL);

	uint16_t height = screen_pSub->valueint / 8;

	screen_pSub = cJSON_GetObjectItem(screen_param_dev,"type_color");

	return_val_if_fail(screen_pSub, NULL);

	uint8_t type_color = screen_pSub->valueint;

	screen_pSub = cJSON_GetObjectItem(screen_param_dev, "polar_data");

	return_val_if_fail(screen_pSub, NULL);

	uint8_t polar_data = screen_pSub->valueint;

	screen_pSub = cJSON_GetObjectItem(screen_param_dev, "polar_oe");

	return_val_if_fail(screen_pSub, NULL);

	uint8_t polar_oe = screen_pSub->valueint;

	screen_pSub = cJSON_GetObjectItem(screen_param_dev, "type_scan");

	return_val_if_fail(screen_pSub, NULL);

	uint8_t type_scan = screen_pSub->valueint;

	screen_pSub = cJSON_GetObjectItem(cjsonRoot, "sno");

	return_val_if_fail(screen_pSub, NULL);

	uint32_t sno = screen_pSub->valueint;

	screen_pSub = cJSON_GetObjectItem(cjsonRoot,"ids_dev");

	uint8_t *ids_dev = screen_pSub->valuestring;

	BinMessage *bm_ret = bin_message_new();

	return_val_if_fail(bm_ret, NULL);

	bm_ret->binMess = (uint8_t*)malloc(sizeof(uint8_t)*1024);
	if(!bm_ret->binMess)
	{
		bin_message_destroy(bm_ret);
		return NULL;
	}

	bm_ret->binMess[0] = 0x00;
	bm_ret->binMess[1] = 0x00;
	bm_ret->binMess[2] = 0x00;

	bm_ret->binMess[3] = 0x50;

	bm_ret->binMess[4] = width & 0xff;
	bm_ret->binMess[5] = width >> 8;

	bm_ret->binMess[6] = height & 0xff;
	bm_ret->binMess[7] = height >> 8;

	bm_ret->binMess[8] = type_color;
	bm_ret->binMess[9] = polar_data;
	bm_ret->binMess[10] = polar_oe;
	bm_ret->binMess[11] = type_scan;

	bm_ret->binLen = 12;

//	bm_ret->oldSno = sno;

//	bm_ret->newSno = ++GSNO;

	GSNO = sno;

	uint32_t ids_dev_len = strlen(ids_dev);

	bm_ret->devIds = (uint8_t*)malloc(sizeof(uint8_t)*ids_dev_len + 1 );
	if(bm_ret->devIds == NULL)
	{
		bin_message_destroy(bm_ret);
		return NULL;
	}

	memcpy(bm_ret->devIds, ids_dev, ids_dev_len);

	bm_ret->devLen = ids_dev_len;

	bm_ret->type = 0;

	return bm_ret;
}

//开关屏幕json解析
BinMessage * switch_cfg_set(cJSON* cJsonRoot)
{
	return_val_if_fail(cJsonRoot, NULL);

	cJSON *switch_cmd = cJSON_GetObjectItem(cJsonRoot, "cmd");

	return_val_if_fail(switch_cmd, NULL);

	cJSON *switch_power = cJSON_GetObjectItem(switch_cmd, "power");

	return_val_if_fail(switch_power, NULL);

	cJSON *switch_psub = cJSON_GetObjectItem(switch_power, "type");

	return_val_if_fail(switch_psub, NULL);

	uint8_t switch_type = switch_psub->valueint;

	uint8_t *switch_time = NULL;

	uint32_t time_flag = 0;

	cJSON *time = cJSON_GetObjectItem(switch_power, "time");
	if(!time)
	{
		time_flag = 0;
	}
	else
	{
		time_flag = 1;

		switch_time = switch_psub->valuestring;
	}

	cJSON *cjson_sno = cJSON_GetObjectItem(cJsonRoot, "sno");

	return_val_if_fail(cjson_sno, NULL);

	uint32_t sno = cjson_sno->valueint;

	cJSON *cjson_ids_dev = cJSON_GetObjectItem(cJsonRoot, "ids_dev");

	return_val_if_fail(cjson_ids_dev, NULL);

	uint8_t *ids_dev = cjson_ids_dev->valuestring;

	BinMessage *bin_ret = bin_message_new();

	return_val_if_fail(bin_ret, NULL);

	bin_ret->binMess = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);
	if(!bin_ret->binMess)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	if(time_flag == 0)
	{
		bin_ret->binMess[0] = 0x00;

		bin_ret->binMess[1] = 0x00;

		bin_ret->binMess[2] = 0x00;

		bin_ret->binMess[3] = 0x52;

		bin_ret->binMess[4] = switch_type;

		bin_ret->binLen = 5;
	}
	else if(time_flag == 1)
	{
		uint32_t hh_open, mm_open, hh_close, mm_close;

		if(split_time_string(time->valuestring,
					&hh_open, &mm_open, &hh_close, &mm_close) < 0)
		{
			return NULL;
		}

		bin_ret->binMess[0] = 0x00;

		bin_ret->binMess[1] = 0x00;

		bin_ret->binMess[2] = 0x00;

		bin_ret->binMess[3] = 0x52;

		bin_ret->binMess[4] = switch_type;

		bin_ret->binMess[5] = hh_open;

		bin_ret->binMess[6] = mm_open;

		bin_ret->binMess[7] = hh_close;

		bin_ret->binMess[8] = mm_close;

		bin_ret->binLen = 9;
	}

	uint32_t ids_dev_len = strlen(ids_dev);

	bin_ret->devIds = (uint8_t*)malloc(sizeof(uint8_t)*ids_dev_len);
	if(!bin_ret->devIds)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	memcpy(bin_ret->devIds, ids_dev, ids_dev_len);

	bin_ret->devLen += ids_dev_len;

	bin_ret->type = 0;

//	bin_ret->newSno = ++GSNO;

//	bin_ret->oldSno = sno;

	GSNO = sno;

	return bin_ret;
}

//设置亮度json解析
BinMessage * light_cfg_set(cJSON* cJsonRoot)
{
	
	return_val_if_fail(cJsonRoot, NULL);

	cJSON *cjson_sno = cJSON_GetObjectItem(cJsonRoot, "sno");

	return_val_if_fail(cjson_sno, NULL);

	cJSON *cjson_dev_list = cJSON_GetObjectItem(cJsonRoot, "ids_dev");

	return_val_if_fail(cjson_dev_list, NULL);

	uint8_t *ids_dev = cjson_dev_list->valuestring;

	cJSON *light_cmd = cJSON_GetObjectItem(cJsonRoot, "cmd");

	return_val_if_fail(light_cmd, NULL);

	cJSON *light_light = cJSON_GetObjectItem(light_cmd, "light");

	return_val_if_fail(light_light, NULL);

	cJSON *light_psub = cJSON_GetObjectItem(light_light, "type");

	return_val_if_fail(light_psub, NULL);

	uint8_t light_type = light_psub->valueint;

	BinMessage *bin_ret = bin_message_new();

	return_val_if_fail(bin_ret, NULL);

	bin_ret->binMess = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);
	if(!bin_ret->binMess)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	if(light_type == 0 || light_type == 2)
	{
		light_psub = cJSON_GetObjectItem(light_light, "value_fix");
		if(!light_psub)
		{
			bin_message_destroy(bin_ret);

			return NULL;
		}


		uint32_t value_fix = light_psub->valueint;

		bin_ret->binMess[0] = 0x00;

		bin_ret->binMess[1] = 0x00;

		bin_ret->binMess[2] = 0x00;

		bin_ret->binMess[3] = 0x49;

		bin_ret->binMess[4] = light_type;

		bin_ret->binMess[5] = value_fix;

		bin_ret->binLen = 6;
	}
	else if(light_type == 1)
	{
		light_psub = cJSON_GetObjectItem(light_light, "value_period");
		if(!light_psub)
		{
			bin_message_destroy(bin_ret);

			return NULL;
		}

		uint32_t val_period_size = cJSON_GetArraySize(light_psub);

		uint32_t i, hour = 0, min = 0, length = 0;

		uint8_t time_buf[40], value_buf[20];

		memset(time_buf, 0, 40);

		memset(value_buf, 0, 20);

		for(i = 0; i < val_period_size; i++)
		{
			
			cJSON *p_sub = cJSON_GetArrayItem(light_psub, i);

			cJSON *time =  cJSON_GetObjectItem(p_sub, "time");
			if(!time)
			{
				bin_message_destroy(bin_ret);

				return NULL;
			}

			split_time(time->valuestring, &hour, &min);

			time_buf[0+2*i] = hour;

			time_buf[1+2*i] = min;

			value_buf[i] = cJSON_GetObjectItem(p_sub, "value")->valueint;

			length++;
		}

		bin_ret->binMess[0] = 0x00;

		bin_ret->binMess[1] = 0x00;

		bin_ret->binMess[2] = 0x00;

		bin_ret->binMess[3] = 0x49;

		bin_ret->binMess[4] = light_type;

		bin_ret->binLen = 7;

		memcpy(bin_ret->binMess + bin_ret->binLen, time_buf, 2*length);

		bin_ret->binLen += 2*length;

		memcpy(bin_ret->binMess + bin_ret->binLen, value_buf, length);

		bin_ret->binLen += length;
	}
	else
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	uint32_t ids_dev_len = strlen(ids_dev);

	bin_ret->devIds = (uint8_t*)malloc(sizeof(uint8_t)*ids_dev_len);
	if(!bin_ret->devIds)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	memcpy(bin_ret->devIds, ids_dev, ids_dev_len);

	bin_ret->devLen += ids_dev_len;

//	bin_ret->newSno = ++GSNO;

//	bin_ret->oldSno = cjson_sno->valueint;

	GSNO = cjson_sno->valueint;

	bin_ret->type = 0;

	return bin_ret;

}

//设置网络参数json解析
BinMessage * net_cfg_set(cJSON* cJsonRoot)
{
	return_val_if_fail(cJsonRoot, NULL);

	cJSON *net_cmd = cJSON_GetObjectItem(cJsonRoot, "cmd");

	return_val_if_fail(net_cmd, NULL);

	cJSON *net_param_net = cJSON_GetObjectItem(net_cmd, "param_net");

	return_val_if_fail(net_param_net, NULL);

	cJSON *net_psub = cJSON_GetObjectItem(net_param_net, "type");

	return_val_if_fail(net_psub, NULL);

	uint8_t net_type = net_psub->valueint;

	net_psub = cJSON_GetObjectItem(net_param_net, "ip");

	return_val_if_fail(net_psub, NULL);

	uint8_t *ip = net_psub->valuestring;

	net_psub = cJSON_GetObjectItem(net_param_net, "gateway");

	return_val_if_fail(net_psub, NULL);

	uint8_t *gateway = net_psub->valuestring;

	net_psub = cJSON_GetObjectItem(net_param_net, "ip_svr");

	return_val_if_fail(net_psub, NULL);

	uint8_t *ip_svr = net_psub->valuestring;

	net_psub = cJSON_GetObjectItem(net_param_net, "port_svr");

	return_val_if_fail(net_psub, NULL);

	uint8_t port_svr = net_psub->valueint;

	net_psub = cJSON_GetObjectItem(net_param_net, "sec_hb");

	return_val_if_fail(net_psub, NULL);

	uint8_t sec_hb = net_psub->valueint;

	net_psub = cJSON_GetObjectItem(net_param_net, "mask");

	return_val_if_fail(net_psub, NULL);

	uint8_t *mask = net_psub->valuestring;

	cJSON *cjson_sno = cJSON_GetObjectItem(cJsonRoot, "sno");

	return_val_if_fail(cjson_sno, NULL);

	uint32_t sno = cjson_sno->valueint;

	cJSON *cjson_ids_dev = cJSON_GetObjectItem(cJsonRoot, "ids_dev");

	return_val_if_fail(cjson_ids_dev, NULL);

	uint8_t *ids_dev = cjson_ids_dev->valuestring;

	BinMessage *bin_ret = bin_message_new();

	return_val_if_fail(bin_ret, NULL);

	bin_ret->binMess = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);
	if(!bin_ret->binMess)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	bin_ret->binMess[0] = 0x00;

	bin_ret->binMess[1] = 0x00;

	bin_ret->binMess[2] = 0x00;

	bin_ret->binMess[3] = 0x4d;

	bin_ret->binMess[4] = net_type;

	int one, two, three, four;

	if(split_net_param_string(ip, &one, &two, &three, &four) < 0)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	bin_ret->binMess[5] = one;

	bin_ret->binMess[6] = two;

	bin_ret->binMess[7] = three;

	bin_ret->binMess[8] = four;

	if(split_net_param_string(ip_svr, &one, &two, &three, &four) < 0)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	bin_ret->binMess[9] = one;

	bin_ret->binMess[10] = two;

	bin_ret->binMess[11] = three;

	bin_ret->binMess[12] = four;

	if(split_net_param_string(ip_svr,&one,&two,&three,&four) < 0)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	bin_ret->binMess[13] = one;

	bin_ret->binMess[14] = two;

	bin_ret->binMess[15] = three;

	bin_ret->binMess[16] = four;

	bin_ret->binMess[17] = port_svr & 0xff;

	bin_ret->binMess[18] = port_svr >> 8;

	bin_ret->binMess[19] = sec_hb & 0xff;

	bin_ret->binMess[20] = sec_hb >> 8;

	if(split_net_param_string(mask,&one,&two,&three,&four) < 0)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	bin_ret->binMess[21] = one;

	bin_ret->binMess[22] = two;

	bin_ret->binMess[23] = three;

	bin_ret->binMess[24] = four;

	bin_ret->binLen = 25;

	uint32_t dev_ids_len = strlen(ids_dev);

	bin_ret->devIds = (uint8_t*)malloc(sizeof(uint8_t)*dev_ids_len);
	if(!bin_ret->devIds)
	{
		bin_message_destroy(bin_ret);

		return NULL;
	}

	memcpy(bin_ret->devIds, ids_dev, dev_ids_len);

	bin_ret->devLen = dev_ids_len;

//	bin_ret->newSno = ++GSNO;

//	bin_ret->oldSno = sno;

	GSNO = sno;

	return bin_ret;
}

//色之gprs参数 暂时没有完成
BinMessage * gprs_cfg_set(cJSON* cJsonRoot)
{
	
}


//参数的获取
BinMessage * param_cfg_get(cJSON* cJsonRoot)
{
	return_val_if_fail(cJsonRoot, NULL);

	cJSON *cmd = cJSON_GetObjectItem(cJsonRoot, "cmd");

	return_val_if_fail(cmd, NULL);

	cJSON *cmd_psub = cJSON_GetObjectItem(cmd, "get");

	return_val_if_fail(cmd_psub, NULL);

	uint32_t get_cmd_no = 0, flag = 0;

	if(strcmp("param_dev", cmd_psub->valuestring) == 0)
	{
		get_cmd_no = 0x50;
	}
	else if(strcmp("param_net", cmd_psub->valuestring) == 0)
	{
		get_cmd_no = 0x5a;
	}
	else if(strcmp("param_gprs", cmd_psub->valuestring) == 0)
	{
		get_cmd_no = 0x4d;
	}
	else if(strcmp("power", cmd_psub->valuestring) == 0)
	{
		get_cmd_no = 0x52;
	}
	else if(strcmp("light", cmd_psub->valuestring) == 0)
	{
		get_cmd_no = 0x49;
	}
	else
	{
		return NULL;
	}

	cmd_psub = cJSON_GetObjectItem(cJsonRoot, "sno");

	return_val_if_fail(cmd_psub, NULL);

	uint32_t sno = cmd_psub->valueint;

	cmd_psub = cJSON_GetObjectItem(cJsonRoot, "ids_dev");

	return_val_if_fail(cmd_psub, NULL);

	uint8_t* ids_dev = cmd_psub->valuestring;

	BinMessage *bm_ret = bin_message_new();

	return_val_if_fail(bm_ret, NULL);

	bm_ret->binMess = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);
	if(!bm_ret->binMess)
	{
		bin_message_destroy(bm_ret);

		return NULL;
	}

	bm_ret->binMess[0] = 0x02;

	bm_ret->binMess[1] = 0x00;

	bm_ret->binMess[2] = 0x00;

	bm_ret->binMess[3] = get_cmd_no;

	bm_ret->binLen = 4;

//	bm_ret->newSno = ++GSNO;

//	bm_ret->oldSno = sno;

	GSNO = sno;

	uint32_t ids_dev_len = strlen(ids_dev);

	bm_ret->devIds = (uint8_t*)malloc(sizeof(uint8_t)*ids_dev_len);
	if(!bm_ret->devIds)
	{
		bin_message_destroy(bm_ret);

		return NULL;
	}

	memcpy(bm_ret->devIds, ids_dev, ids_dev_len);

	bm_ret->devLen += ids_dev_len;

	bm_ret->type = 0;

	return bm_ret;
}

