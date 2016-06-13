

BinMessage* bin_message_new();

void bin_message_destroy(BinMessage* bm);

MmManage * mm_manage_new();

void* mm_manage_destroy(MmManage* mm);

int memory_manage(MmManage* mm, uint8_t* data, uint32_t dataLen);

ParamContent* param_content_new();

void* param_content_destroy(ParamContent* pc);
