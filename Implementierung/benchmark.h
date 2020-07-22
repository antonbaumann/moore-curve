#include <stdint.h>

// run benchmark of all supported implementations `repetitions` times with `degree`
// writes result of last repetition to benchmark directory if `write_result` is true
void benchmark(uint32_t degree, uint32_t repetitions, uint32_t write_results);