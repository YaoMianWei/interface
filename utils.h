
#include"interface.h"

int split_time_string(uint8_t* timeString, int* openHour, int* openMin, int* closHour, int* closeMin);

int split_time(uint8_t* timeString, int* hour, int* min);

int split_net_param_string(uint8_t* timeString, int* one, int* two, int* three, int* four);

int escape_sequence(unsigned char* buf, int startPos, int endPos, int len);

void delete_char(char* srcString, char delChar1, char delChar2);

uint32_t match_color(uint32_t color);

int week_ctr(cJSON* week);


