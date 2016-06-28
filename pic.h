

#ifndef _PIC_H_
#define _PIC_H_

#include<stdio.h>
#include<SDL/SDL.h>
#include<SDL/SDL_ttf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FontInfo
{
	int size;
	char* type;
	int style[4];
	SDL_Color color;
}FontInfo;

typedef struct _PartContent
{
	int width;
	int height;
}PartContent;

int save_bmp_unicode(uint16_t* text, int textLen, const FontInfo* FontInfo, const PartContent* pc);

#ifdef __cplusplus
}
#endif

#endif 
