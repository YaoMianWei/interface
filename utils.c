#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"interface.h"

int split_time_string(uint8_t* timeString, int* openHour, int* openMin, int* closHour, int* closeMin)
{
	return_val_if_fail(timeString, -1);

	char openTime[20];

	char closeTime[20];

	sscanf(timeString, "%[0-9,:] - %[0-9,:]", openTime, closeTime);

#if BYTEDEBUG
	printf("openTime=%s,closeTime=%s", openTime, closeTime);
#endif

	sscanf(openTime, "%d:%d", openHour, openMin);

	sscanf(closeTime, "%d:%d", closHour, closeMin);

#if BYTEDEBUG
	printf("openHour%d,openMin=%d,closHour=%d,closeMin=%d",*openHour,*openMin,*closHour,*closeMin);
#endif

	return 1;
}

int split_time(uint8_t* timeString, int* hour, int* min)
{
	int ret = 0;

	return_val_if_fail(timeString, -1);

	ret = sscanf(timeString, "%d:%d", hour, min);
	if(ret != 2)
	{
		return -1;
	}

	return 1;
}

int split_net_param_string(uint8_t* timeString, int* one, int* two, int* three, int* four)
{
	int ret = 0;

	return_val_if_fail(timeString, -1);

	ret = sscanf(timeString, "%d.%d.%d.%d", one, two, three, four);
	if(ret != 4)
	{

		return -1;
	}

	return 1;
}

