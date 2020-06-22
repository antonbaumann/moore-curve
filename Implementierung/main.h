void print_help();

int moore_asm(long degree, uint64_t *x, uint64_t *y);

int moore_c_recursive(long degree, uint64_t *x, uint64_t *y);

long parse_degree(char *);

int write_svg(char *filename, uint64_t *x, uint64_t *y, unsigned int degree);