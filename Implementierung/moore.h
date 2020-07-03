void moore_c_iterative(uint64_t degree, uint64_t *x, uint64_t *y);

// extern function name must start with underscore if compiled on mac
#ifdef __APPLE__
extern void moore(uint64_t degree, uint64_t *x, uint64_t *y);
#else
extern void _moore(uint64_t degree, uint64_t *x, uint64_t *y);
extern void moore(uint64_t degree, uint64_t *x, uint64_t *y) {
    _moore(degree, x, y);
}
#endif