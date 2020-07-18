void moore_c_iterative(uint32_t degree, uint32_t *x, uint32_t *y);
void moore_c_batch(uint32_t degree, uint32_t *x, uint32_t *y);


extern void moore(uint32_t degree, uint32_t *x, uint32_t *y);

#ifdef __AVX2__
extern void moore_avx(uint32_t degree, uint32_t *x, uint32_t *y);
#endif