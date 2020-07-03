void print_help();

void moore_asm(long degree, uint64_t *x, uint64_t *y);

long parse_degree(char *);

int write_svg(char *filename, uint64_t *x, uint64_t *y, unsigned int degree);