
#include"interface.h"

#ifdef __cplusplus
extern "C" {
#endif

int add_head_tail(uint8_t* dataBuf, uint32_t dataLen);

int get_time_info(cJSON* cJsonRoot, uint8_t* timeBuf);

int get_part_info(cJSON* cJsonRoot, unsigned char* partBuf);

int get_property(cJSON* cJsonRoot, uint8_t* proBuf);

void * get_animate_info(cJSON* cJsonRoot, uint32_t* type, uint32_t* speed, uint32_t* time);

uint8_t * get_profits(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc);

uint8_t * get_bmp(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc);

uint8_t * get_CTH_text(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc);

uint8_t * get_text_audio(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc);

uint8_t * get_commom_text(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc);

uint8_t * get_content(cJSON* cJsonRoot, uint32_t* contentLen);


#if 0
//该函数获取每个节目中单个分区的文本或者图片内容 目前暂时支持四种
uint8_t * (*get_every_part_content[])(cJSON* cJsonRoot, uint32_t type, uint32_t* contentLen, ParamContent* pc) = 
{
	get_commom_text,  //处理普通文本的函数地址
	
	get_bmp, //处理bmp函数地址
	
	get_CTH_text, //处理时间文本 倒计时 温湿度函数地址

	get_profits //处理字幕的函数地址
};
#endif

#ifdef __cplusplus
}
#endif


