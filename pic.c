

#include"pic.h"

//将图层复制到另一个图层的x，y位置
static void apply_surface(int x, int y, SDL_Surface* pSrc, SDL_Surface* pDest)
{
	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = pSrc->w;
	rect.h = pSrc->h;

	SDL_BlitSurface(pSrc, NULL, pDest, &rect);
}

//打开SDL字体
static TTF_Font* create_font(const FontInfo* fontInfo)
{
	TTF_Font *font = NULL;
	if(TTF_Init() < 0)
	{
		printf("TTF INIT FAILED\n");

		return NULL;
	}

	font = TTF_OpenFont(fontInfo->type, fontInfo->size);
	if(font == NULL)
	{
		printf("TTF OPEN FAILED\n");

		return NULL;
	}

	return font;
}

//创建图层
static SDL_Surface* create_surface(int BMPWidth, int BMPHeight)
{
	SDL_Surface *pScreen;

	pScreen = SDL_CreateRGBSurface(0, BMPWidth, BMPHeight, 24, 0, 0, 0, 0);
	if(pScreen == NULL)
	{
		printf("SDL CREATERGB FAILED\n");

		return NULL;
	}

	return pScreen;
}

//查找分区能够放的最多文本的文本结束位置
static int find_part_end_pos_unicode(uint16_t* text, int textLen, int startPos,
		TTF_Font* font, int partWidth)
{
	if(startPos > textLen)
	{
		return -1;
	}
	
	uint16_t *tmp = text + startPos;

	int minx = 0, maxx = 0, miny = 0, maxy = 0, advance = 0, subStrLen = 0;

	while(*tmp != '\0')
	{
		TTF_GetFontStyle(font);

		TTF_GlyphMetrics(font, *tmp, &minx, &maxx, &miny, &maxy, &advance);

		subStrLen += advance;

		if(subStrLen >= partWidth)
		{
			break;
		}

		startPos++;

		tmp++;
	}
	
	return startPos;
}

//获取bmp图片的宽
static int get_BMP_width_unicode(uint16_t* text, int textLen, TTF_Font* font, int partWidth)
{
	int count = 0;

	int startPos = 0, endPos = 0;

	while(startPos < textLen - 1)
	{
		endPos = find_part_end_pos_unicode(text, textLen, startPos, font, partWidth);

		printf("endpos:%d\n",endPos);

		if(endPos != -1)
		{
			count++;
			
			startPos = endPos;
		}
	}

	int BMPWidthRet = count * partWidth;

	return BMPWidthRet;
}

//获取字体的宽和高
static void get_font_WH_unicode(uint16_t* text, TTF_Font* font, int* w, int* h)
{
	TTF_SizeUNICODE(font, text, w, h);
}

//计算BMP图片展示的具体位置
static void BMP_show_position(uint16_t* text, TTF_Font* font, 
		const PartContent* pc, int* x, int* y)
{
	int fontWidth = 0, fontHeight = 0;

	get_font_WH_unicode(text, font, &fontWidth, &fontHeight);

	int partWidth = pc->width;

	int partHeight = pc->height;

	*x = (partWidth-fontWidth)/2;

	*y = (partHeight-fontHeight)/2;
}

//将文字处理成图片并保存在当前目录下./unicode.bmp
int save_bmp_unicode(uint16_t* text, int textLen,
		const FontInfo* fontInfo, const PartContent* pc)
{
	TTF_Font *font = create_font(fontInfo);
	if(font == NULL)
	{
		return -1;
	}
	
	int BMPWidth = get_BMP_width_unicode(text, textLen, font, pc->width);

	int BMPHeight = pc->height;

	SDL_Surface *pScreen = create_surface(BMPWidth, BMPHeight);
	if(pScreen == NULL)
	{
		return -1;
	}

	int startPos = 0, endPos = 0, xPos = 0;

	while(startPos < textLen - 1)
	{
		endPos = find_part_end_pos_unicode(text, textLen, startPos, font, pc->width);

		int subStrLen = endPos - startPos;

		uint16_t subStr[subStrLen + 1];

		memset(subStr, 0, subStrLen + 1);

		memcpy(subStr, text + startPos, subStrLen * sizeof(uint16_t));

		subStr[subStrLen] = '\0';

		SDL_Color RGB_Red = {255, 0, 0};

		SDL_Surface *pText = TTF_RenderUNICODE_Solid(font, subStr, RGB_Red);
		if(pText != NULL)
		{
//			SDL_SaveBMP(pText, "substr.bmp");

			int xOffset = 0, yOffset = 0;

			BMP_show_position(subStr, font, pc, &xOffset, &yOffset);

			apply_surface(xPos + xOffset, yOffset, pText, pScreen);

			xPos += pc->width;

			SDL_FreeSurface(pText);
		}

		startPos = endPos;
	}

	SDL_SaveBMP(pScreen, "unicode.bmp");

	TTF_CloseFont(font);

	TTF_Quit();

	SDL_FreeSurface(pScreen);

	SDL_Quit();

	return 1;
}

