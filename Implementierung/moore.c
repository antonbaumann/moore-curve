#include <stdint.h>

// makes returning two variables more comfortable
struct tuple {
    uint64_t x, y;
};


struct tuple rotate(struct tuple coord, uint64_t length, uint64_t top, uint64_t right) {
    struct tuple new_coord;
    if (!top) {
        if (right) {
            new_coord.x = (length - 1) - coord.y;
            new_coord.y = (length - 1) - coord.x;
        } else {
            new_coord.x = coord.y;
            new_coord.y = coord.x;
        }
    } else new_coord = coord;
    return new_coord;
}


struct tuple hilbert_coord_at_index(uint64_t index, uint64_t degree) {
    struct tuple coord = {.x=0, .y=0};
    uint64_t max_iterations = (uint64_t) 2 << (2 * degree - 1);

    for (uint64_t i = 1; i < max_iterations; i *= 2) {
        uint64_t right = (uint64_t) 1 & (index / 2);
        uint64_t top = (uint64_t) 1 & (index ^ right);

        coord = rotate(coord, i, top, right);
        if (right) coord.x += i;
        if (top) coord.y += i;

        index >>= (uint64_t) 2;
    }

    if (degree % 2 != 0) {
        struct tuple new = {.x=coord.y, .y=coord.x};
        return new;
    }
    return coord;
}

struct tuple moore_coord_at_index(
        uint64_t index,
        uint64_t degree,
        uint64_t max_iterations
) {
    if (degree == 1) {
        return hilbert_coord_at_index(index,1);
    }

    uint64_t hilbert_max_iterations = max_iterations / 4;
    uint64_t hilbert_index = index % hilbert_max_iterations;
    uint64_t hilbert_side_length = (uint64_t) 2 << (degree - 2);

    struct tuple coord = hilbert_coord_at_index(
            hilbert_index,
            degree - 1
    );

    uint64_t quadrant = index / hilbert_max_iterations;

    uint64_t tmp;

    switch (quadrant) {
        case 0:
            tmp = coord.x;
            coord.x = hilbert_side_length - coord.y - 1;
            coord.y = tmp;
            break;
        case 1:
            tmp = coord.x;
            coord.x = hilbert_side_length - coord.y - 1;
            coord.y = tmp + hilbert_side_length;
            break;
        case 2:
            tmp = coord.x;
            coord.x = coord.y + hilbert_side_length;
            coord.y = 2 * hilbert_side_length - tmp - 1;
            break;
        case 3:
            tmp = coord.x;
            coord.x = coord.y + hilbert_side_length;
            coord.y = hilbert_side_length - tmp - 1;
            break;
    }

    return coord;
}

void moore_c_iterative(uint64_t degree, uint64_t *x, uint64_t *y) {
    uint64_t shifts = 2 * degree - 1;
    uint64_t max_iterations = (uint64_t) 2 << shifts; // 2 ^ (2n)

    for (uint64_t i = 0; i < max_iterations; i++) {
        struct tuple coord = moore_coord_at_index(i, degree, max_iterations);
        x[i] = coord.x;
        y[i] = coord.y;
    }
}