
#ifdef WINDOWS  

#include <windows.h>

#include <stdio.h>

#include <ctype.h>

#else

#include <iconv.h>
#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#endif

#define LEN 10

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


void bm_print(BinMessage *bm)
{
	printf("Something about bm:\n");
	printf("The bin'length is %d\n", bm->binLen);
	printf("The message is this:\n");
	int i;
	for(i = 0; i < bm->binLen; i++)
	{
#if 1
		printf("0x%02X,", bm->binMess[i]);
#else
		printf("%02X  ",bm->binMess[i]);
#endif
	}
	printf("\n");
	printf("The devId's length is %d\n", bm->devLen);
	printf("The devId is this:\n");
	for(i = 0; i < bm->devLen; i++)
	{
		printf("%02X  ", bm->devIds[i]);
	}
	printf("\n");
}

void buf_print(uint8_t* buf, int length)
{
	printf("The buf' length is %d\n", length);
	printf("buf is this:\n");
	int i;
	for(i = 0; i < length; i++)
	{
		printf("%02X  ",buf[i]);
	}
	printf("\n");
}


//转义函数
int escape_sequence(unsigned char* buf, int startPos, int endPos, int len)
{
	unsigned char tmp[MALLOCSIZE];

	memset(tmp, 0, MALLOCSIZE);

	int n, countRet = 0;

	int i = 0;
	while(i < len)
	{
		tmp[i] = buf[i];

		i++;
	}

	i = 0;
	while(i < len)
	{
		i++;
	}

	int pos = startPos;

	while(pos < len - endPos)
	{
		if(tmp[pos] != 0xA5 && tmp[pos] != 0xaa && tmp[pos] != 0xae)
		{
			pos++;
		}
		else
		{
			if(tmp[pos] == 0xa5)
			{
				buf[pos + countRet] = 0xaa;

				buf[pos + countRet + 1] = 0x05;
			}
			else if(tmp[pos] == 0xaa)
			{
				buf[pos + countRet] = 0xaa;

				buf[pos + countRet + 1] = 0x0a;
			}
			else if(tmp[pos] == 0xae)
			{
				buf[pos + countRet] = 0xaa;

				buf[pos + countRet + 1] = 0x0e;
			}
			else
			{
				return 0;
			}

			pos++;

			countRet++;
		}


		n = pos;

		while(n < len)
		{
			buf[n + countRet] = tmp[n];
			n++;
		}
	}

	return countRet;
}

void delete_char(char* srcString, char delChar1, char delChar2)
{
	char *p = srcString;

	char *q = srcString;

	while(*q)
	{
		if((*q != delChar1) && (*q != delChar2))
		{
			*p++ = *q;
		}

		q++;
	}

	*p = '\0';
}

uint32_t match_color(uint32_t color)
{
	uint32_t color_ret = 0x01;

	switch(color)
	{
		case 255: 
			color = 0x01;
			break;

		case 65280: 
			color = 0x02;
			break;

		case 65535:
			color = 0x03;
			break;

		case 16711680:
			color = 0x04;
			break;

		case 16711935:
			color = 0x05;
			break;

		case 16776960:
			color = 0x06;
			break;

		case 16777215:
			color = 0x07;
			break;
	}

	return color;
}

int week_ctr(cJSON* week)
{
	int ret = 1;

	int week_size = cJSON_GetArraySize(week);

	int i, n = 0;

	cJSON *pSub;

	for(i = 0; i < week_size; i++)
	{
		pSub = cJSON_GetArrayItem(week, i);

		n = 1 << pSub->valueint;

		ret += n;
	}

	return ret;
}

size_t replace_substr_to_deststr(char* ctrString, char* srcString, char* destString)
{
	size_t number = 1;

	size_t countRet = 0;

	char *p, *q, *x, *y, *z;

	size_t srcStrLen = strlen(srcString);

	size_t destStrLen = strlen(destString);

	size_t remainStrLen = 0;

	int i;

	while(*ctrString)
	{
		p = ctrString;

		q = srcString;

		while((*p == *q) && (*p != '\0') && (*q != '\0'))
		{
			p++;

			q++;

		}


		if(*q == '\0')
		{
			countRet++;

			remainStrLen = strlen(p)+1;

			char buf[remainStrLen];

			memset(buf, 0, remainStrLen);

			for(i = 0; i < remainStrLen-1; i++)
			{
				buf[i] = *(p+i);
			}

			y = p - srcStrLen;

			z = destString;

			while(*z != '\0')
			{
				*y++ = *z;

				z++;
			}

			x = buf;

			while(*x != '\0')
			{
				*y++ = *x;

				x++;

			}

			number = 0;
#if 0
			if(destStrLen > srcStrLen)
			{
				number = destStrLen-1;
			}
			else
			{
				number = srcStrLen - destStrLen + 1; 
			}
#endif
			*y = '\0';

		}

		*ctrString++; //杩欓噷鍙互浼樺寲
		//		ctrString = ctrString + number;
	}

	*ctrString = '\0'; 

	return countRet;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0)
		return -1;

	memset(outbuf,0,outlen);
	if (iconv(cd,pin,(size_t* __restrict__)(&inlen),pout,(size_t* __restrict__)(&outlen))==-1)
		return -1;

	iconv_close(cd);
	return 0;
}

int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

int b2u(char *inbuf,int inlen,char *outbuf,int outlen)
{
	return code_convert("BIG5","utf-8",inbuf,inlen,outbuf,outlen);
}

int u2b(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	return code_convert("utf-8","BIG5",inbuf,inlen,outbuf,outlen);
}


int FUTF82WConvert( const char* a_szSrc, wchar_t* a_szDest, int a_nDestSize )
{
#ifdef WINDOWS  
	return MultiByteToWideChar(CP_UTF8, 0, a_szSrc, -1, a_szDest, a_nDestSize );
#else
	size_t result;
	
	iconv_t env;
	
	int size = strlen(a_szSrc)+1 ;
	
	env = iconv_open("WCHAR_T","UTF-8");
	if (env==(iconv_t)-1)
	{
		return -1;
	}
	
	result = iconv(env,(char**)&a_szSrc,(size_t*)&size,(char**)&a_szDest,(size_t*)&a_nDestSize); 
	if (result==(size_t)-1)
	{
		return -1;
	}
	
	iconv_close(env);
	
	return (int)result;
#endif
}
