
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include "interface.h"

BinMessage* bin_message_new()
{
	BinMessage *bm_ret = NULL;
	
	bm_ret = (BinMessage*)malloc(sizeof(BinMessage));
	if(!bm_ret)
	{
		return NULL;
	}

	bm_ret->binLen = 0;
	bm_ret->devLen = 0;
	bm_ret->newSno = 0;
	bm_ret->oldSno = 0;
	bm_ret->type = 0;

	return bm_ret;
	
}

void bin_message_destroy(BinMessage* bm)
{
	if(bm == NULL)
	{
		return;
	}

	if(bm->binMess != NULL)
	{
		free(bm->binMess);
	}

	if(bm->devIds != NULL)
	{
		free(bm->devIds);
	}
	free(bm);
}
