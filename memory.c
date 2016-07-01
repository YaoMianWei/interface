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

MmManage * mm_manage_new()
{
	MmManage *mm = NULL;

	mm = (MmManage*)malloc(sizeof(MmManage)); 

	return_val_if_fail(mm, NULL);

	mm->data = (uint8_t*)malloc(sizeof(uint8_t)*MALLOCSIZE);

	if(!mm->data)
	{
		free(mm);

		return NULL;
	}

	mm->dataLen = 0;

	mm->mmSize = MALLOCSIZE;

	return mm;
}

void* mm_manage_destroy(MmManage* mm)
{
	return_val_if_fail(mm, NULL);

	if(mm->data)
	{
		free(mm->data);
	}

	free(mm);
}

int memory_manage(MmManage* mm, uint8_t* data, uint32_t dataLen)
{
	return_val_if_fail(mm,-1);

	if(mm->dataLen + dataLen >= mm->mmSize)
	{
		uint8_t tmp_buf[mm->dataLen];

		memset(tmp_buf, 0, mm->dataLen);

		memcpy(tmp_buf, mm->data, mm->dataLen);

		free(mm->data);


		mm->data = NULL;


		mm->mmSize = mm->mmSize + (((dataLen / MALLOCSIZE) + 1) * MALLOCSIZE);


		mm->data = (uint8_t*)malloc(sizeof(uint8_t)*(mm->mmSize));

		
		return_val_if_fail(mm->data, -1);


		memcpy(mm->data, tmp_buf, mm->dataLen);

		memcpy(mm->data + mm->dataLen, data, dataLen);


		mm->dataLen += dataLen;

	}
	else
	{
		memcpy(mm->data + mm->dataLen, data, dataLen);

		mm->dataLen += dataLen;
	}

	return 1;
}

ParamContent* param_content_new()
{
	ParamContent *pc_ret = NULL;

	pc_ret = (ParamContent*)malloc(sizeof(ParamContent));

	return_val_if_fail(pc_ret, NULL);

	return pc_ret;
}

void* param_content_destroy(ParamContent* pc)
{
	return_val_if_fail(pc, NULL);

	free(pc);
}


