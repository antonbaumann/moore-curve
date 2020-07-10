#include <stdint.h>

// makes returning two variables more comfortable
struct tuple {
    uint64_t x, y;
};

// rotates/modifies shape for all 4 quadrants
struct tuple rotate(struct tuple coord, uint64_t length, uint64_t top, uint64_t right) {
    struct tuple new_coord;
    if (!top) {                                     // the two top one ones are the same shape
        if (right) {                                // and the bottom shapes are mirrored on a diagonal axis
            new_coord.x = (length - 1) - coord.y;   // they are moved accordingly as well
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

    for (uint64_t i = 1; i < max_iterations; i *= 2) {  // i *= 2,  simulates a binary shit to the left
        uint64_t right = (uint64_t) 1 & (index / 2);    // to decide what Quadrant look up the 2 least significant Bits of i, for ex. 7 = 0b01(11)
        uint64_t top = (uint64_t) 1 & (index ^ right);  // first see if on left or right Half by checking the first of those deciding Bits
                                                        // secondly check the second of those two if in the right or left corner
        coord = rotate(coord, i, top, right);           // 00 -> bottom-left
        if (right) coord.x += i;                        // 01 -> top-left
        if (top) coord.y += i;                          // 10 -> top-right
                                                        // 11 -> bottom-right
        index >>= (uint64_t) 2;
    }

    if (degree % 2 != 0) {
        struct tuple new = {.x=coord.y, .y=coord.x};
        return new;
    }
    return coord;
}
// constructs moore curve out of 4 * (n-1) degree Hilbert-Curves
struct tuple moore_coord_at_index(
        uint64_t index,
        uint64_t degree,
        uint64_t max_iterations
) { // deg 1 Hilbert-Curve = deg 1 Moore-Curve
    if (degree == 1) {
        return hilbert_coord_at_index(index,1);
    }
    // converting moore_Index to hilbert_Index, the Hilbert-Curve (degree = n-1) fits exactly 4 times into Moore-Curve (degree = n)
    uint64_t hilbert_max_iterations = max_iterations / 4;
    uint64_t hilbert_index = index % hilbert_max_iterations;
    uint64_t hilbert_side_length = (uint64_t) 2 << (degree - 2);

    struct tuple coord = hilbert_coord_at_index(
            hilbert_index,
            degree - 1
    );

    uint64_t quadrant = index / hilbert_max_iterations;

    uint64_t tmp;

    switch (quadrant) {                                   // each Switchstatement is according to one quadrant
        case 0:                                           // which is filled with a hilbert curve and rotated
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
    uint64_t max_iterations = (uint64_t) 2 << shifts; // 2 ^ (2n) , each shift == 2* in binary we use this to iterate over the index in binary for ex. 7 == 0b0111
                                                      // pass max_iterations which equals amount of points 2^(2n) = 4^n
    for (uint64_t i = 0; i < max_iterations; i++) {
        struct tuple coord = moore_coord_at_index(i, degree, max_iterations);
        x[i] = coord.x;
        y[i] = coord.y;
    }
}