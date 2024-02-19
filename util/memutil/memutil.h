void _memset(void* addr, int v, size_t len);
void _memcpy(void* v1, void* v2, size_t len);
int search_memory(int current, int end, BYTE* bytes, size_t len);
int patch_memory(int start, int end, BYTE* search_for, BYTE* patched, size_t len);
void __nop(unsigned int start, unsigned int end);
void __jmp(unsigned int off, unsigned int loc);
void __call(unsigned int off, unsigned int loc);
void XUNLOCK(void* addr, size_t len);