void print_help();

void moore_asm_wrapper(uint32_t degree, uint32_t *x, uint32_t *y);
void moore_c_naive_wrapper(uint32_t degree, uint32_t *x, uint32_t *y);
void moore_c_batch_wrapper(uint32_t degree, uint32_t *x, uint32_t *y);

uint32_t parse_uint32(char *);

int write_svg(char *filename, uint32_t *x, uint32_t *y, unsigned int degree);