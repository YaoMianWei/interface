#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"utils.h"
#include"mul.h"
#include"pic.h"
#include"memory.h"
#include"base64.h"

uint8_t * (*get_every_part_content[])(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc) = 
{
	get_commom_text,  //处理普通文本的函数入口
	
	get_bmp, //处理bmp函数入口
	
	get_CTH_text, //处理时间文本 倒计时 温湿度函数入口

	get_profits, //处理字幕的函数入口
	
	get_text_audio  //处理语音的函数入口
};

//时钟文本替换{YYYY} {MM} {DD} {hh} {mm} {ss} {w}
static void clock_text_ctr(uint8_t* text)
{
	uint8_t year[5]= {0x32, 0x30, 11, 12};

	uint8_t month[3] = {13, 14};

	uint8_t date[3] = {15, 16};

	uint8_t hour[3] = {17, 18};

	uint8_t min[3] = {19, 20};

	uint8_t sec[3] = {23, 24};

	uint8_t week[3] = {21, 22};

	replace_substr_to_deststr(text, "{YYYY}", year);

	replace_substr_to_deststr(text, "{MM}", month);

	replace_substr_to_deststr(text, "{DD}", date);

	replace_substr_to_deststr(text, "{hh}", hour);

	replace_substr_to_deststr(text, "{mm}", min);

	replace_substr_to_deststr(text, "{ss}", sec);

	replace_substr_to_deststr(text, "{w}", week);
}

//倒计时替换{T}和{N}
static void timing_text_ctr(uint8_t* text, char* dest)
{

	replace_substr_to_deststr(text, "{T}", dest);

	uint8_t N[5] = {0x1c, 0x1d, 0x1e, 0x1f};

	replace_substr_to_deststr(text, "{N}", N);
}

//问世读替换{T}和{H}
static void humidtemp_text_ctr(uint8_t* text)
{
	uint8_t T[4] = {1,2,3};

	uint8_t H[3] = {4,5};

	replace_substr_to_deststr(text, "{T}", T);

	replace_substr_to_deststr(text, "{H}", H);
}


//增加包头和包尾以及包的长度
int add_head_tail(uint8_t* dataBuf, uint32_t dataLen)
{
	uint32_t length = 0;

	uint8_t tmp_buf[dataLen];

	memset(tmp_buf, 0, dataLen);

	memcpy(tmp_buf, dataBuf, dataLen);

	int length_pos = length;

	dataBuf[length] = 0x00;

	length++;

	dataBuf[length] = 0x00;

	length++;

	uint32_t head_pos = length;

	dataBuf[length] = 0xa5;

	length++;

	dataBuf[length] = 0x03;

	length++;

	dataBuf[length] = 0x7b;

	length++;

	int package_len_pos = length;

	dataBuf[length] = 0x00;

	length++;

	dataBuf[length] = 0x00;

	length++;

	dataBuf[length] = GSNO & 0XFF;

	length++;

	dataBuf[length] = GSNO >> 8;

	length++;

	dataBuf[length] = GSNO >> 16;

	length++;

	dataBuf[length] = GSNO >> 24;

	length++;

	dataBuf[length] = 0x00;

	length++;

	dataBuf[length] = 0x00;

	length++;

	memcpy(dataBuf + length, tmp_buf, dataLen);

	length += dataLen;

	dataBuf[package_len_pos] = (length+3-2) & 0xff;

	dataBuf[package_len_pos + 1] = (length+3-2) >> 8;

	int i, check_sum = 0;

	for(i = head_pos; i < length ; i++)
	{
		check_sum += dataBuf[i];	
	}

	dataBuf[length] = check_sum & 0xff;

	length++;

	dataBuf[length] = check_sum >> 8;

	length++;

	dataBuf[length] = 0xAE;

	length++;

	int add_len = escape_sequence(dataBuf, 3, 1, length);

	length += add_len;
	
	dataBuf[length_pos] = (length-2) & 0xff;

	dataBuf[length_pos + 1] = (length-2) >> 8;

#if debug 
	buf_print(dataBuf, length);
#endif
	return length;
}

//获取时间消息
int get_time_info(cJSON* cJsonRoot, uint8_t* timeBuf)
{
	int length = 0;

	int date_pos = length;

	int ret = -1;

	timeBuf[length] = 0;

	length++;

	timeBuf[length] = 1;

	length++;

	timeBuf[length] = 1;

	length++;

	timeBuf[length] = 99;

	length++;

	timeBuf[length] = 12;

	length++;

	timeBuf[length] = 31;

	length++;

	int week_pos = length;

	timeBuf[length] = 0xff;

	length++;

	int period_pos = length;

	timeBuf[length] = 0x00;

	length++;

	cJSON *cjson_time = cJSON_GetObjectItem(cJsonRoot, "info_period_pro");

	return_val_if_fail(cjson_time, length);

	cJSON *cjson_psub = cJSON_GetObjectItem(cJsonRoot, "week");
	if(cjson_psub)
	{
		int ret = week_ctr(cjson_psub);

		timeBuf[week_pos] = ret;
	}

	cjson_psub = cJSON_GetObjectItem(cJsonRoot, "period");
	if(cjson_psub)
	{
		int period_size = cJSON_GetArraySize(cjson_psub);

		timeBuf[period_pos] = period_size;

		int period_pos, hh_open = 0, mm_open = 0, hh_close = 0, mm_close = 0;

		for(period_pos = 0; period_pos < period_size; period_pos++)
		{
			cJSON *cjson_item = cJSON_GetArrayItem(cjson_psub, period_pos);	

			ret = sscanf(cjson_item->valuestring, "%d:%d-%d:%d", &hh_open, 
					&mm_open, &hh_close, &mm_close);
			if(ret != 4)
			{
				return length;   //!!!!这里可能会有bug!
			}
			

			timeBuf[length] = hh_open;

			length++;

			timeBuf[length] = mm_open;

			length++;

			timeBuf[length] = hh_close;

			length++;

			timeBuf[length] = mm_close;

			length++;
		}
	}

	return length;
}


int get_part_info(cJSON* cJsonRoot, unsigned char* partBuf)
{

	cJSON *cjson_property = cJSON_GetObjectItem(cJsonRoot, "property_pro");

	return_val_if_fail(cjson_property,-1);

	cJSON *cjson_width = cJSON_GetObjectItem(cjson_property, "width");

	return_val_if_fail(cjson_width, -1);

	cJSON *cjson_height = cJSON_GetObjectItem(cjson_property, "height");

	return_val_if_fail(cjson_height, -1);

	int length = 0;

	partBuf[length] = 0x01;

	length++;

	partBuf[length] = 0x00;

	length++;

	partBuf[length] = 0;

	length++;

	partBuf[length] = 0;

	length++;

	partBuf[length] = 0;

	length++;

	partBuf[length] = 0;

	length++;

	partBuf[length] = (cjson_width->valueint/8) & 0xff;

	length++;

	partBuf[length] = (cjson_width->valueint/8) >> 8;

	length++;

	partBuf[length] = (cjson_height->valueint/8) & 0xff;

	length++;

	partBuf[length] = (cjson_height->valueint/8) >> 8;

	length++;

	cJSON *list_region = cJSON_GetObjectItem(cJsonRoot, "list_region");

	return_val_if_fail(list_region, length);

	length = 0;

	int region_size = cJSON_GetArraySize(list_region);

	partBuf[length] = region_size;

	length++;

	int region_pos;

	for(region_pos = 0; region_pos < region_size; region_pos++)
	{

		cJSON *cjson_psub = cJSON_GetArrayItem(list_region, region_pos);

		cJSON *cjson_wait = cJSON_GetObjectItem(cjson_psub, "type_wait");

		if(cjson_wait)
		{
			partBuf[length] = cjson_wait->valueint;
		}
		else
		{
			partBuf[length] = 0x00;;
		}
		
		length++;

		cJSON *info_pos = cJSON_GetObjectItem(cjson_psub, "info_pos");
		if(info_pos)
		{
			cJSON *cjson_x = cJSON_GetObjectItem(info_pos, "x");
			if(cjson_x)
			{
				partBuf[length] = (cjson_x->valueint/8) & 0xFF;

				length++;

				partBuf[length] = (cjson_x->valueint/8) >> 8;

				length++;
			}

			cJSON *cjson_y = cJSON_GetObjectItem(info_pos, "y");
			if(cjson_y)
			{
				partBuf[length] = (cjson_y->valueint/8) & 0xff;

				length++;

				partBuf[length] = (cjson_y->valueint/8) >> 8;

				length++;
			}

			cJSON *cjson_w = cJSON_GetObjectItem(info_pos, "w");
			if(cjson_w)
			{
				partBuf[length] = (cjson_w->valueint/8) & 0xff;

				length++;

				partBuf[length] = (cjson_w->valueint/8) >> 8;

				length++;
			}

			cJSON *cjson_h = cJSON_GetObjectItem(info_pos, "h");
			if(cjson_h)
			{
				partBuf[length] = (cjson_h->valueint/8) & 0xff;

				length++;

				partBuf[length] = (cjson_h->valueint/8) >> 8;

				length++;
			}

		}
		else
		{
			partBuf[length] = 0;

			length++;

			partBuf[length] = 0;

			length++;

			partBuf[length] = 0;

			length++;

			partBuf[length] = 0;

			length++;

			partBuf[length] = (cjson_width->valueint/8) & 0xff;

			length++;

			partBuf[length] = (cjson_width->valueint/8) >> 8; 

			length++;	

			partBuf[length] = (cjson_height->valueint/8) & 0xff;

			length++;

			partBuf[length] = (cjson_height->valueint/8) >> 8; 

			length++;
		}

		cJSON *list_item = cJSON_GetObjectItem(cjson_psub, "list_item");

		return_val_if_fail(list_item, -1);

		int list_item_count = cJSON_GetArraySize(list_item);

		partBuf[length] = list_item_count;

		length++;
	}

#if debug
	buf_print(partBuf, length);
#endif
	return length;	
}


//获取属性
int get_property(cJSON* cJsonRoot, uint8_t* proBuf)
{
	int length = 0;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x4A;

	length++;

	uint32_t pro_id_pos = length;

	proBuf[length] = 0x05;

	length++;

	proBuf[length] = 0x00;

	length++;

	int play_pos = length;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x01;

	length++;

	proBuf[length] = 0x00;

	length++;

	int time_pos = length;

	cJSON *cjson_pro_id = cJSON_GetObjectItem(cJsonRoot, "id_pro");

	return_val_if_fail(cjson_pro_id, -1);

	proBuf[pro_id_pos] = cjson_pro_id->valueint + 4;

	cJSON *cjson_property = cJSON_GetObjectItem(cJsonRoot, "property_pro");

	return_val_if_fail(cjson_property, -1);

	cJSON *cjson_psub = cJSON_GetObjectItem(cjson_property, "width");

	return_val_if_fail(cjson_psub, -1);

	cjson_psub = cJSON_GetObjectItem(cjson_property, "height");

	return_val_if_fail(cjson_psub, -1);

	cjson_psub = cJSON_GetObjectItem(cjson_property, "model_loop");
	if(cjson_psub)
	{
		proBuf[play_pos] = 0x00;

		proBuf[play_pos+1] = cjson_psub->valueint & 0xff;

		proBuf[play_pos+2] = cjson_psub->valueint >> 8;

	}

	cjson_psub = cJSON_GetObjectItem(cjson_property, "model_fixed_time");
	if(cjson_psub)
	{
		proBuf[play_pos] = 0x01;

		proBuf[play_pos++] = cjson_psub->valueint & 0xff;

		proBuf[play_pos++] = cjson_psub->valueint >> 8;

	}

	cjson_psub = cJSON_GetObjectItem(cjson_property, "model_cmd");
	if(cjson_psub)
	{
		proBuf[play_pos] = 0x02;

		proBuf[play_pos++] = cjson_psub->valueint & 0xff;

		proBuf[play_pos++] = cjson_psub->valueint >> 8;

	}

	uint8_t time_buf[512] = {0};

	int time_buf_len = get_time_info(cjson_property, time_buf);

	memcpy(proBuf + time_pos, time_buf, time_buf_len);

	length += time_buf_len;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x00;

	length++;

	proBuf[length] = 0x00;

	length++;

	uint8_t part_buf[512] = {0};

	int part_buf_len = get_part_info(cJsonRoot, part_buf);

	if(part_buf_len == -1)
	{
		return -1;
	}

	memcpy(proBuf + length, part_buf, part_buf_len);

	length += part_buf_len;

#if debug
	printf("属性:");

	buf_print(proBuf, length);
#endif

	return length;
}

//获取动画的信息
void* get_animate_info(cJSON* cJsonRoot, uint32_t* type, uint32_t* speed, uint32_t* time)
{
	cJSON *model_normal = cJSON_GetObjectItem(cJsonRoot, "model_normal");

	if(model_normal)
	{
		*type = model_normal->valueint;
	}
	else
	{
		*type = 0x01;
	}

	cJSON *cjson_speed = cJSON_GetObjectItem(cJsonRoot, "speed");
	if(cjson_speed)
	{
		*speed = cjson_speed->valueint;
	}
	else
	{
		*speed = 0x0a;
	}

	cJSON *cjson_time = cJSON_GetObjectItem(cJsonRoot, "time_stay");
	if(cjson_time)
	{
		*time = cjson_time->valueint;
	}
	else
	{
		*time = 0x0a;
	}

}

//获取图片的宽和高
static int get_bmp_width_heigth(FILE *fd, int len)
{
	int len_ret = 0;

	int i;

	for(i = 0; i < len; ++i)
	{
		len_ret += (fgetc(fd) << 8 * i);
	}

	return len_ret;
}

//矩阵处理
static void buildData_matrix_graphic(unsigned char *ApData, unsigned char *ApGraphic, int w, int h, int iColorType)
{
	unsigned char pix8 =0;
	int x, y, i = 0;
	if(iColorType >= 1)
	{
		for(y = h-1; y >= 0; --y)
			for(x = 0; x < w; ++x)
			{
				if(ApGraphic[(y * w + x) * 3 + 2] > 127)
					pix8 += 1;
				++i;
				if(8 == i)
				{
					i = 0;					
					*ApData++ = pix8;
					pix8 = 0;
				}
				else
				{
					pix8 = pix8<<1;				
				}
			}
	}
	pix8 = 0;
	if(iColorType >= 2)
	{
		for(y = h-1; y >= 0; --y)
			for(x = 0; x < w; ++x)
			{	
				if(ApGraphic[(y * w + x) * 3 + 1] > 127)
					pix8 += 1;

				++i;
				if(8 == i)
				{
					i = 0;					
					*ApData++ = pix8;
					pix8 = 0;
				}
				else
					pix8 = pix8<<1;
			}
	}
	pix8 = 0;
	if(iColorType >= 3)
	{
		for(y = h - 1; y >= 0; --y)
			for(x = 0; x < w; ++x)
			{
				if(ApGraphic[(y * w + x) * 3 + 0] > 127)
					pix8 += 1;

				++i;
				if(8 == i)
				{
					i = 0;					
					*ApData++ = pix8;
					pix8 = 0;
				}
				else
					pix8 = pix8<<1;
			}
	}
}

//获取图片的有效信息
static uint8_t* get_bmp_effect_data(cJSON* cJsonRoot, char* filePath, ParamContent* pc, uint32_t* bmpConentLen)
{
	return_val_if_fail(filePath, NULL);

	FILE *fd = NULL;

	fd = fopen(filePath, "rb");

	return_val_if_fail(fd, NULL);

	fseek(fd, 18, SEEK_SET);

	int bmp_width = get_bmp_width_heigth(fd, 4);

	int bmp_height = get_bmp_width_heigth(fd, 4);

	uint32_t part_width = pc->partWidth, part_height = pc->partHeight;

	uint32_t count_page = pc->countPage;

	if(part_width == 0 || part_height == 0)
	{
		part_width = bmp_width;

		part_height = bmp_height;
	}

	printf("get_bmp_effect_data\n");

	if(bmp_width == 0 || bmp_height == 0)
	{
		printf("bmp_width == 0 || bmp_height == 0\n");
		return NULL;
	}

	if(part_height > bmp_height && part_width > bmp_width)
	{
		printf("part_height > bmp_height && part_width > bmp_width\n");
		return NULL;
	}

	uint32_t len_page = part_width * part_height * 3;

	uint32_t len_cur = count_page * len_page;

	uint32_t len_all = 0;

	if(part_width != bmp_width)
	{
		len_all = len_page * (bmp_width / part_width + (bmp_width % part_width > 0 ? 1 : 0)) * (bmp_height / part_height);
	}
	else
	{
		len_all = len_page * (bmp_width / part_width) * (bmp_height / part_height + (bmp_height % part_height > 0 ? 1 : 0));
	}

	uint32_t bmp_data_len = len_all;

	uint8_t bmp_data[bmp_data_len];

	memset(bmp_data, 0, bmp_data_len);

	fseek(fd, 28, SEEK_SET);

	uint32_t bmp32_flag = fgetc(fd) == 0x20;

	fseek(fd, 54, SEEK_SET);

	if(part_width == bmp_width && part_height == bmp_height)
	{
		if(!bmp32_flag)
		{
			fread(bmp_data + len_cur, sizeof(uint8_t), len_page, fd);
		}
		else
		{
			uint32_t i;

			for(i = 0; i < bmp_width * bmp_height; i++)
			{
				fseek(fd, 54 + 4 * i, SEEK_SET);

				fread(bmp_data + len_cur + i * 3, 1, 3, fd);
			}
		}

		count_page++;
	}
	else if(part_width != bmp_width)
	{
		int y, pos = 0;

		int len_width = part_width * 3;

		int len_width_src = bmp_width * 3;

		int mod_4 = len_width_src % 4;

		len_width_src = (mod_4 == 0) ? len_width_src : (len_width_src + 4 - mod_4);

		while(pos < bmp_width)
		{
			int len_read = bmp_width - pos;

			if(len_read > part_width)
			{
				len_read = part_width;
			}

			len_read *= 3;

			for(y = 0; y < part_height; y++)
			{
				fseek(fd, 54 + y * len_width_src + pos * 3, SEEK_SET);

				fread(bmp_data + y * len_width + len_cur, 1, len_read, fd);
			}


			count_page++;

			len_cur += len_page;

			pos += part_width;
		}
	}
	else if(part_height != bmp_height)
	{
		int pos = bmp_height;

		while(pos >= part_height)
		{
			fseek(fd, 54 + (pos - part_width) * bmp_width * 3, SEEK_SET);

			fread(bmp_data + len_cur, 1, len_page, fd);

			count_page++;

			len_cur += len_page;

			pos -= part_height;
		}


	}

	fclose(fd);

	uint32_t part_color_type = pc->partColorType;

	int page_matrix_len = part_height * (part_width / 8) * part_color_type;

	int sum_matrix_len = page_matrix_len * count_page;

	int package_count = sum_matrix_len / 512;

	if(0 != sum_matrix_len % 512)
	{
		package_count++;
	}

	uint8_t matrix_buf[sum_matrix_len];

	memset(matrix_buf, 0, sum_matrix_len);

	int count_data_page_graphic = part_height * part_width * 3;

	int i;

	for(i = 0; i < count_page; i++)
	{
		buildData_matrix_graphic(matrix_buf + i * page_matrix_len, bmp_data +i * count_data_page_graphic, part_width, part_height, part_color_type);
	}

	uint8_t* bmp_content_ret = (uint8_t*)malloc(sizeof(uint8_t)*package_count*MALLOCSIZE);

	return_val_if_fail(bmp_content_ret, NULL);

	uint32_t anmi_type = 1, anmi_speed = 10, anmi_time = 3;
#if 1
	cJSON *info_animate = cJSON_GetObjectItem(cJsonRoot, "info_animate");
	if(info_animate)
	{
		get_animate_info(info_animate, &anmi_type, &anmi_speed, &anmi_time);
	}
#endif
	uint8_t data_buf[1024] = {0};

	uint8_t length = 0;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x4a;

	length++;

	uint32_t pro_id_pos = length;

	data_buf[length] = pc->proId + 4;

	length++;

	data_buf[length] = 0x05;

	length++;

	data_buf[length] = pc->partPos+1;

	length++;

	data_buf[length] = pc->itemSum;

	length++;

	data_buf[length] = pc->itemPos+1;

	length++;

	data_buf[length] = package_count & 0xff;

	length++;

	data_buf[length] = package_count >> 8;

	length++;

	uint32_t package_pos = length;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = count_page;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = anmi_type;

	length++;

	data_buf[length] = anmi_speed;;

	length++;

	data_buf[length] = anmi_time;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	data_buf[length] = 0x00;

	length++;

	uint32_t bmp_pos = length;

	int len_sum = 0, package_len = 0, copy_len = sum_matrix_len;

	int last_matrix_package_len = sum_matrix_len % 512;

	uint8_t tmp_buf[MALLOCSIZE];

	for(i = 0; i < package_count; i++)
	{
		data_buf[package_pos] = (i) & 0xff;

		data_buf[package_pos + 1] = (i) >> 8;

		if(i < package_count)
		{
			uint32_t tmp_length = 0;

			memset(tmp_buf, 0, MALLOCSIZE);

			memcpy(tmp_buf, data_buf, bmp_pos);

			tmp_length += bmp_pos;

			memcpy(tmp_buf + bmp_pos, matrix_buf + i * 512, 512);

			tmp_length += 512;

			package_len = add_head_tail(tmp_buf, tmp_length);

			memcpy(bmp_content_ret + len_sum, tmp_buf, package_len);

			len_sum += package_len;

			buf_print(bmp_content_ret, len_sum);
		}
		else
		{
			uint32_t tmp_length = 0;

			memset(tmp_buf, 0, MALLOCSIZE);

			memcpy(tmp_buf, data_buf, bmp_pos);

			tmp_length += bmp_pos;

			memcpy(tmp_buf + bmp_pos, matrix_buf + i * 512, last_matrix_package_len);

			tmp_length += last_matrix_package_len;

			package_len = add_head_tail(tmp_buf, tmp_length);

			memcpy(bmp_content_ret + len_sum, tmp_buf, package_len);

			len_sum += package_len;

			buf_print(bmp_content_ret, len_sum);
		}
	}

	*bmpConentLen = len_sum;

#if debug
	buf_print(bmp_content_ret, len_sum);
#endif
	return bmp_content_ret;
}


//获取字幕
uint8_t* get_profits(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc)
{
	return_val_if_fail(cJsonRoot, NULL);

	cJSON *cjson_profits = cJSON_GetObjectItem(cJsonRoot, "text");

	return_val_if_fail(cjson_profits, NULL);

	uint32_t utf_text_len = strlen(cjson_profits->valuestring);

	char* utf_text = cjson_profits->valuestring;

#if 0
	uint32_t gb_text_len = 2 * utf_text_len;

	char gb_text[gb_text_len];

	memset(gb_text, 0, gb_text_len);

	u2g(utf_text, utf_text_len, gb_text, gb_text_len);
#endif

	int unicode_text_len = 5 * utf_text_len;

	wchar_t unicode[unicode_text_len];

	memset(unicode, 0, unicode_text_len);

	FUTF82WConvert(utf_text, unicode, unicode_text_len);

	int unicode_len = wcslen(unicode);

	int i;

	uint16_t buf[unicode_len + 1];

	memset(buf, 0, unicode_len + 1);

	for(i = 0; i < unicode_len; i++)
	{
		buf[i] = unicode[i];
	}

	buf[i] = '\0';

	SDL_Color RGB_Red = { 255, 0, 0 };

	FontInfo font_info = {.size = 20, .type = "./font/FZDBSJW.TTF", .color = RGB_Red};

	PartContent part_content = {.width = 128, .height = 32};

	int ret = save_bmp_unicode(buf, unicode_len + 1, &font_info, &part_content);
	if(ret < 0)
	{
		return NULL;
	}

	uint32_t bmp_content_len = 0;

	uint8_t* content_ret = get_bmp_effect_data(cJsonRoot, "./unicode.bmp", pc, &bmp_content_len);

	return_val_if_fail(content_ret, NULL);

	*contentLen = bmp_content_len;

	return content_ret;
}

uint8_t* get_bmp(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc)
{
	printf("get_bmp1\n");

	return_val_if_fail(cJsonRoot, NULL);

	cJSON *cjson_bmp = cJSON_GetObjectItem(cJsonRoot, "data_bmp");

	return_val_if_fail(cjson_bmp, NULL);

	int net_bmp_len = strlen(cjson_bmp->valuestring);

//	printf("cjson_bmp->valuestring : %s\n", cjson_bmp->valuestring);

	delete_char(cjson_bmp->valuestring, '\r', '\n');

	net_bmp_len = strlen(cjson_bmp->valuestring);

	printf("cjson_bmp->valuestring:%s\n",cjson_bmp->valuestring);

	printf("net_bmp_len : %d\n", net_bmp_len);

	FILE *fp = NULL;

	fp = fopen("1.bmp", "wb");
	if(!fp)
	{
		printf("can't open 1.bmp\n");

		return NULL;
	}
	
	printf("get_bmp2\n");

#if 0
	uint8_t *decode_bmp_data = NULL;

	int  decode_bmp_len = 0;

	decode_bmp_data = base64_decode(cjson_bmp->valuestring, net_bmp_len, &decode_bmp_len);

	printf("\n");
#endif
	char org_string[net_bmp_len];

	memset(org_string, 0, net_bmp_len);

	int de_len = Base64Decode(org_string, cjson_bmp->valuestring, net_bmp_len, 1);

	
	printf("de_len: %d\n",de_len);

	fwrite(org_string, sizeof(char), de_len, fp);

	fclose(fp);

	printf("get_bmp3\n");

	uint32_t bmp_content_len = 0;

	uint8_t* bmp_content_ret = get_bmp_effect_data(cJsonRoot, "1.bmp", pc, &bmp_content_len);

	return_val_if_fail(bmp_content_ret, NULL);

	*contentLen = bmp_content_len;

	return bmp_content_ret;
}

uint8_t* get_CTH_text(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc)
{
	return_val_if_fail(cJsonRoot, NULL);

	uint8_t *content = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);

	return_val_if_fail(content, NULL);

	uint32_t size = 0, color = 0;

	cJSON *cjson_size = cJSON_GetObjectItem(cJsonRoot, "size");
	if(cjson_size)
	{
		size = cjson_size->valueint/8;
	}
	else
	{
		size = 0x02;
	}

	cJSON *cjson_color = cJSON_GetObjectItem(cJsonRoot, "color");
	if(cjson_color)
	{
		color = cjson_color->valueint;

		color = match_color(color);
	}
	else
	{
		color = 0x01;
	}

	uint32_t anmi_type = 1, anmi_speed = 10, anmi_time = 3;

	cJSON *info_animate = cJSON_GetObjectItem(cJsonRoot, "info_animate");
	if(info_animate)
	{
		get_animate_info(info_animate, &anmi_type, &anmi_speed, &anmi_time);
	}

	cJSON *cjson_text = cJSON_GetObjectItem(cJsonRoot, "text");

	uint32_t text_len = strlen(cjson_text->valuestring);

	char utf_text[2*text_len];

	memset(utf_text, 0, 2*text_len);

	memcpy(utf_text, cjson_text->valuestring, text_len);

	uint32_t text_type = 0;

	cJSON* cjson_type = cJSON_GetObjectItem(cJsonRoot, "type_item");

	text_len = strlen(utf_text);

	uint8_t gb_text[2*text_len];

	memset(gb_text, 0, 2*text_len);

	u2g(utf_text, text_len, gb_text, 2*text_len);

	uint32_t gb_text_len = strlen(gb_text);

	int year = 0, month = 0, date = 0, isReserve = 0;

	if(strcmp(cjson_type->valuestring, "text_clock") == 0)
	{
		text_type = 2;
		clock_text_ctr(gb_text);
	}
	if(strcmp(cjson_type->valuestring, "text_temp_humid") == 0)
	{
		text_type = 4;

		humidtemp_text_ctr(gb_text);

		buf_print(gb_text, strlen(gb_text));
	}
	if(strcmp(cjson_type->valuestring, "text_timing") == 0)
	{
		text_type = 3;

		cJSON* param_timing = cJSON_GetObjectItem(cJsonRoot, "param_timing");

		return_val_if_fail(param_timing, NULL);

		cJSON* cjson_target = cJSON_GetObjectItem(param_timing, "target");

		cJSON* cjson_isReserve = cJSON_GetObjectItem(param_timing, "isReserve");

		return_val_if_fail(cjson_target, NULL);

		return_val_if_fail(cjson_isReserve, NULL);

		isReserve = cjson_isReserve->valueint;

		timing_text_ctr(gb_text, cjson_target->valuestring);

		int ret = sscanf(cjson_target->valuestring, "%d-%d-%d", &year, &month, &date);
		if(ret != 3)
		{
			return NULL;
		}
	}



	gb_text_len = strlen(gb_text);

	uint32_t length = 0;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x4a;

	length++;

	content[length] = pc->proId + 4;

	length++;

	content[length] = text_type;

	length++;

	content[length] = pc->partPos + 1;

	length++;

	content[length] = pc->itemSum;

	length++;

	content[length] = pc->itemPos + 1;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = size;

	length++;

	content[length] = color;

	length++;

	content[length] = anmi_type;

	length++;

	content[length] = anmi_speed;

	length++;

	content[length] = anmi_time;

	length++;

	content[length] = 0x00;

	length++;

	int obligate_flag = length;

	if(text_type == 3)
	{
		content[length] = year%2000;

		length++;

		content[length] = month;

		length++;

		content[length] = date;

		length++;

		content[length] = isReserve;

		length++;

		content[length] = 0x00;

		length++;
	}
	else
	{
		content[length] = 0x00;

		length++;

		content[length] = 0x00;

		length++;

		content[length] = 0x00;

		length++;

		content[length] = 0x00;

		length++;

		content[length] = 0x00;

		length++;
	}

	if(gb_text_len >= 255)
	{
		memcpy(content+length, gb_text, 255);

		length += 255;
	}
	else
	{
		memcpy(content+length, gb_text, gb_text_len);

		length += gb_text_len;
	}


	uint32_t len_sum = add_head_tail(content, length);

	*contentLen = len_sum;

#if debug
	buf_print(content, len_sum);
#endif
	return content;
}

uint8_t* get_text_audio(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc)
{
	return_val_if_fail(cJsonRoot, NULL);

	uint8_t *content = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);

	return_val_if_fail(content, NULL);

	uint32_t size = 0, color = 0;

	cJSON* cjson_size = cJSON_GetObjectItem(cJsonRoot, "size");
	if(cjson_size)
	{
		size = cjson_size->valueint;
	}
	else
	{
		size = FONTSIZE;
	}

	cJSON *cjson_color = cJSON_GetObjectItem(cJsonRoot, "color");
	if(cjson_color)
	{
		color = cjson_color->valueint;

		color = match_color(color);
	}
	else
	{
		color = FONTCOLOR;
	}

	uint32_t anmi_type = ANMITYPE, anmi_speed = ANMISPEED, anmi_time = ANMITIME;

	cJSON *info_animate = cJSON_GetObjectItem(cJsonRoot, "info_animate");
	if(info_animate)
	{
		get_animate_info(info_animate, &anmi_type, &anmi_speed, &anmi_time);
	}

	cJSON *text = cJSON_GetObjectItem(cJsonRoot, "text");

	return_val_if_fail(text, NULL);

	uint32_t utf_text_size = strlen(text->valuestring);

	char gb_buf[UTFTOGB * utf_text_size];

	memset(gb_buf, 0, UTFTOGB * utf_text_size);

	u2g(text->valuestring, utf_text_size, gb_buf, UTFTOGB * utf_text_size);

	uint32_t gb_size = strlen(gb_buf);

	cJSON *param_audio = cJSON_GetObjectItem(cJsonRoot, "param_audio");

	return_val_if_fail(param_audio, NULL);

	int speeker = AUDIOSPEEKER, speed = AUDIOSPEED, hint = AUDIOHINT, count_play = AUDIOCOUNTPALY, tone = AUDIOTONE;

	cJSON *pSub = cJSON_GetObjectItem(param_audio, "speaker");

	return_val_if_fail(pSub, NULL);

	speeker = pSub->valueint;

	pSub = cJSON_GetObjectItem(param_audio, "speed");

	return_val_if_fail(pSub, NULL);

	speed = pSub->valueint;

	pSub = cJSON_GetObjectItem(param_audio, "hint");

	return_val_if_fail(pSub, NULL);

	hint = pSub->valueint;

	pSub = cJSON_GetObjectItem(param_audio, "count_play");

	return_val_if_fail(pSub, NULL);

	count_play = pSub->valueint;

	pSub = cJSON_GetObjectItem(param_audio, "tone");

	return_val_if_fail(pSub, NULL);

	tone = pSub->valueint;

	uint32_t length = 0;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x4a;

	length++;

	content[length] = pc->proId + 4;

	length++;

	content[length] = 0x06;

	length++;

	content[length] = pc->partPos+1;

	length++;

	content[length] = pc->itemSum;

	length++;

	content[length] = pc->itemPos+1;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = size;

	length++;

	content[length] = color;

	length++;

	content[length] = anmi_type;

	length++;

	content[length] = anmi_speed;

	length++;

	content[length] = anmi_time;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = speeker;

	length++;

	content[length] = speed;

	length++;

	content[length] = hint;

	length++;

	content[length] = count_play;

	length++;

	content[length] = tone;

	length++;

	memcpy(content + length, gb_buf, strlen(gb_buf));

	if(gb_size >= COMMOMTEXTSIZE)
	{
		memcpy(content + length, gb_buf, COMMOMTEXTSIZE);

		length += COMMOMTEXTSIZE;
	}
	else
	{
		memcpy(content + length, gb_buf, gb_size);

		length += gb_size;
	}

	uint32_t sum_len = add_head_tail(content,length);

	*contentLen = sum_len;
#if debug
	buf_print(content, sum_len);
#endif
	return content;
}


uint8_t* get_commom_text(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc)
{
	return_val_if_fail(cJsonRoot, NULL);

	uint8_t *content = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);

	return_val_if_fail(content, NULL);

	uint32_t size = 0, color = 0;

	cJSON *cjson_size = cJSON_GetObjectItem(cJsonRoot, "size");
	if(cjson_size)
	{
		size = cjson_size->valueint / MOD8;
	}
	else
	{
		size = FONTSIZE;
	}

	cJSON *cjson_color = cJSON_GetObjectItem(cJsonRoot, "color");
	if(cjson_color)
	{
		color = cjson_color->valueint;

		color = match_color(color);	
	}
	else
	{
		color = FONTCOLOR;
	}

	uint32_t anmi_type = ANMITYPE, anmi_speed = ANMISPEED, anmi_time = ANMITIME;

	cJSON *info_animate = cJSON_GetObjectItem(cJsonRoot, "info_animate");
	if(info_animate)
	{
		get_animate_info(info_animate, &anmi_type, &anmi_speed, &anmi_time);
	}

	cJSON *text = cJSON_GetObjectItem(cJsonRoot, "text");

	return_val_if_fail(text, NULL);

	uint32_t utf_text_size = strlen(text->valuestring);

	char gb_buf[UTFTOGB * utf_text_size];

	memset(gb_buf, 0, UTFTOGB * utf_text_size);

	u2g(text->valuestring, utf_text_size, gb_buf, UTFTOGB * utf_text_size);

	uint32_t gb_buf_size = strlen(gb_buf);

	uint32_t length = 0;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x4a;

	length++;

	content[length] = pc->proId + 4;

	length++;

	content[length] = 0x01;

	length++;


	//默认是从0开始，分区位置是从一开始所以加1
	content[length] = pc->partPos + 1;

	length++;

	content[length] = pc->itemSum;

	length++;

	content[length] = pc->itemPos + 1;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = size;

	length++;

	content[length] = color;

	length++;

	content[length] = anmi_type;

	length++;

	content[length] = anmi_speed;

	length++;

	content[length] = anmi_time;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	content[length] = 0x00;

	length++;

	if(gb_buf_size >= COMMOMTEXTSIZE)
	{
		memcpy(content + length, gb_buf, COMMOMTEXTSIZE);

		length += COMMOMTEXTSIZE;
	}
	else
	{
		memcpy(content + length, gb_buf, gb_buf_size);

		length += gb_buf_size;
	}

	uint32_t sum_len = add_head_tail(content,length);

	*contentLen = sum_len;
#if 0
	buf_print(content, sum_len);
#endif
	return content;
}


//将节目序列化 text 1  graphic 2 CTH 3  text_pic 4
static int program_serialize(uint8_t* typeString)
{
	int ret = -1;

	if(strcmp(typeString, "text")==0)
	{
		ret = 0;
	}

	if(strcmp(typeString, "graphic") == 0)
	{
		ret = 1;
	}

	if(strcmp(typeString, "text_clock") == 0 || 
			strcmp(typeString, "text_temp_humid")==0 ||
			strcmp(typeString, "text_timing") == 0)
	{
		ret = 2;
	}

	if(strcmp(typeString, "text_pic") == 0)
	{
		ret = 3;
	}

	if(strcmp(typeString, "text_audio") == 0)
	{
		ret = 4;
	}

	return ret;
}

uint8_t * get_content(cJSON* cJsonRoot, uint32_t* contentLen)
{
	return_val_if_fail(cJsonRoot, NULL);

	cJSON *cjson_pro_id = cJSON_GetObjectItem(cJsonRoot, "id_pro");

	return_val_if_fail(cjson_pro_id, NULL);

	uint32_t pro_id = cjson_pro_id->valueint;

	cJSON *list_region = cJSON_GetObjectItem(cJsonRoot, "list_region");

	return_val_if_fail(list_region, NULL);

	cJSON *property_pro = cJSON_GetObjectItem(cJsonRoot, "property_pro");

	return_val_if_fail(property_pro, NULL);

	cJSON *cjson_width = cJSON_GetObjectItem(property_pro, "width");

	return_val_if_fail(cjson_width, NULL);

	uint32_t width = cjson_width->valueint;

	cJSON *cjson_height = cJSON_GetObjectItem(property_pro, "height");

	return_val_if_fail(cjson_height, NULL);

	uint32_t heigth = cjson_height->valueint;

	uint32_t type_color = 2;
	
	cJSON *cjson_type_color = cJSON_GetObjectItem(property_pro, "type_color");
	if(cjson_type_color)
	{
		type_color = cjson_type_color->valueint;
	}

	uint32_t part_sum, part_pos, item_sum, item_pos;

	part_sum = cJSON_GetArraySize(list_region);

	ParamContent *pc = param_content_new();

	return_val_if_fail(pc, NULL);

	MmManage *mm = mm_manage_new();

	return_val_if_fail(mm, NULL);

	for(part_pos = 0; part_pos < part_sum; part_pos++)
	{
		uint32_t pc_part_width, pc_part_height, pc_part_x, pc_part_y;

		cJSON *region_psub = cJSON_GetArrayItem(list_region, part_pos);

		return_val_if_fail(region_psub, NULL);

		cJSON *info_pos = cJSON_GetObjectItem(region_psub, "info_pos");
		if(info_pos)
		{
			cJSON *w = cJSON_GetObjectItem(info_pos, "w");

			pc_part_width = w->valueint;

			cJSON *h = cJSON_GetObjectItem(info_pos, "h");

			pc_part_height = h->valueint;

			cJSON *x = cJSON_GetObjectItem(info_pos, "x");

			pc_part_x = x->valueint;

			cJSON *y = cJSON_GetObjectItem(info_pos, "y");

			pc_part_y = y->valueint;
		}
		else
		{
			pc_part_x = 0;

			pc_part_y = 0;

			pc_part_width = width;

			pc_part_height = heigth;
		}

		cJSON *list_item = cJSON_GetObjectItem(region_psub, "list_item");

		item_sum = cJSON_GetArraySize(list_item);

		for(item_pos = 0; item_pos < item_sum; item_pos++)
		{
			pc->partSum = part_sum;

			pc->partPos = part_pos;

			pc->itemSum = item_sum;

			pc->itemPos = item_pos;

			pc->partHeight = pc_part_height;

			pc->partWidth = pc_part_width;

			pc->partX = pc_part_x;

			pc->partY = pc_part_y;

			pc->partGifFlag = 0;

			pc->partBorderWidth = 0;

			pc->partColorType = type_color;

			pc->countPage = 0;

			pc->proId = pro_id;

			cJSON *item_psub = cJSON_GetArrayItem(list_item, item_pos);

			pc->itemPos = item_pos;

			cJSON *cjson_type = cJSON_GetObjectItem(item_psub, "type_item");

			int type = program_serialize(cjson_type->valuestring);

			return_val_if_fail(!(type < 0), NULL);

			uint32_t content_len = 0;

			uint8_t *content = NULL;

			content	= get_every_part_content[type](item_psub, type, &content_len, pc);

			return_val_if_fail(content, NULL);

			memory_manage(mm, content, content_len);

			free(content);
		}


	}

	*contentLen = mm->dataLen;

	uint8_t *content_ret = (uint8_t*)malloc(sizeof(uint8_t)*(*contentLen));

	return_val_if_fail(content_ret, NULL);

	memcpy(content_ret, mm->data, mm->dataLen);

	param_content_destroy(pc);

	mm_manage_destroy(mm);

	return content_ret;
}
