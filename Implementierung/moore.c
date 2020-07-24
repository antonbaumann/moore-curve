#include <stdint.h>

// makes returning two variables more comfortable
struct tuple {
    uint64_t x, y;
};

// rotates/modifies shape for all 4 quadrants
struct tuple rotate(struct tuple coord, uint32_t length, uint32_t top, uint32_t right) {
    struct tuple new_coord;
    // the two top quadrants do not have to be mirrored
    if (!top) {
        if (right) {
            // mirror on top-left-to-bottom-right diagonal axis
            new_coord.x = (length - 1) - coord.y;
            new_coord.y = (length - 1) - coord.x;
        } else {
            // mirror on top-right-to-bottom-left diagonal axis
            new_coord.x = coord.y;
            new_coord.y = coord.x;
        }
    } else new_coord = coord;
    return new_coord;
}


struct tuple hilbert_coord_at_index(uint64_t index, uint64_t degree) {
    struct tuple coord = {.x=0, .y=0};
    // i *= 2,  sidelength starts at 1 and is doubled with every iteration
    uint64_t sidelength = (uint64_t) 2 << (degree - 1);
    // to decide what Quadrant look up the 2 least significant Bits of i, for ex. 7 = 0b01(11)
    for (uint64_t i = 1; i < sidelength; i *= 2) {
        // first see if on left or right Half by checking the first of those deciding Bits 1-> right, 0-> left
        uint64_t right = (uint64_t) 1 & (index /2);
        // secondly check if bits are same or different: same -> bottom, different -> top
        uint64_t top = (uint64_t) 1 & (index ^ right);
        // rotate according to previously determined quadrant ( by looking at last two bits)
        coord = rotate(coord, i, top, right);
        // if on the right translate by current sidelength (=i) to the right
        if (right) coord.x += i;
        // if on the top translate by current sidelength (=i) upwards
        if (top) coord.y += i;
        index >>= (uint64_t) 2;
    }
    return coord;
}

struct tuple moore_coord_at_index(
        uint64_t index,
        uint64_t degree,
        uint64_t max_iterations
) { // deg 1 Hilbert-Curve = deg 1 Moore-Curve
    if (degree == 1) {
        return hilbert_coord_at_index(index, 1);
    }
    // converting moore_Index to hilbert_Index,
    // the Hilbert-Curve with degree = n-1 fits exactly 4 times
    // into Moore-Curve with degree = n
    uint64_t hilbert_max_iterations = max_iterations / 4;
    uint64_t hilbert_index = index % hilbert_max_iterations;
    uint64_t hilbert_side_length = (uint64_t) 2 << (degree - 2);

    struct tuple coord = hilbert_coord_at_index(
            hilbert_index,
            degree - 1
    );

    uint64_t quadrant = index / hilbert_max_iterations;

    uint64_t tmp;

    // each Switchstatement is according to one quadrant
    // which is filled with a hilbert curve and rotated
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

void moore_c_iterative(uint32_t degree, uint32_t *x, uint32_t *y) {
    uint64_t shifts = 2 * degree - 1;
    uint64_t max_iterations = (uint64_t) 2 << shifts; // 2^(2*degree)
    for (uint64_t i = 0; i < max_iterations; i++) {
        struct tuple coord = moore_coord_at_index(i, degree, max_iterations);
        x[i] = coord.x;
        y[i] = coord.y;
    }
}

void hilbert_c_iterative(uint32_t degree, uint32_t *x, uint32_t *y) {
    uint64_t shifts = 2 * degree - 1;
    uint64_t max_iterations = (uint64_t) 2 << shifts; // 2^(2*degree)
    for (uint64_t i = 0; i < max_iterations; i++) {
        struct tuple coord = hilbert_coord_at_index(i, degree);
        x[i] = coord.x;
        y[i] = coord.y;
    }
}

// Bottom-Up approach increasing size by *4 each iteration
void hilbert_c_batch(uint32_t degree, uint32_t *x, uint32_t *y) {
    uint64_t quarter = 4;
    uint32_t offset = 2;
    uint64_t tmp = 0;

    for (uint64_t j = 2; j <= degree; j++){
        for (uint64_t i = 0; i < quarter; i++){
            // Starting-Quarter equivalent to Hilbert-Curve for (degree-1) -> Top-Left-Quarter
            y[i + quarter] = offset + y[i];
            x[i + quarter] = x[i];
            // Starting-Quarter -> Bottom-Left-Quarter; x -> y, y -> x
            tmp = x[i];
            x[i] = y[i];
            y[i] = tmp;
            // Top-Right-Quarter -> Top-Left-Quarter
            x[i + 2*quarter] = x[i + quarter] + offset;
            y[i + 2*quarter] = y[i + quarter];
            // Bottom-Left-Quarter -> Bottom-Right-Quarter
            x[i + 3*quarter] = (offset - 1) - x[i] + offset;
            y[i + 3*quarter] = (offset - 1) - y[i];
        }
        quarter <<= 2;
        offset <<= 1;
    }
}

void moore_c_batch(uint32_t degree, uint32_t *x, uint32_t *y) {
    // Hard-Coded first degree
    x[0] = 0; y[0] = 0;
    x[1] = 0; y[1] = 1;
    x[2] = 1; y[2] = 1;
    x[3] = 1; y[3] = 0;

    if (degree == 1) {
        return;
    }

    hilbert_c_batch(degree - 1, x, y);
    // Amount of Points from one Quarter, equivalent to amount of points for (degree-1)Hilbert-Curve
    uint64_t quarter = (uint64_t) 2 << (2 * degree - 3);
    // Sidelength of one Quarter
    uint32_t offset = ((uint32_t) 2 << (degree - 2)) - 1;

    for (uint64_t i = 0; i < quarter; i++) {
        // translate Hilbert -> Top-Left-Moore
        x[i + quarter] = offset - y[i];
        y[i + quarter] = x[i] + offset + 1;
        // translate Hilbert -> Top-Right-Moore
        x[i + 2*quarter] = y[i] + offset + 1;
        y[i + 2*quarter] = 2 * offset - x[i] + 1;
        // copy Top-Right-Moore -> Bottom-Right-Moore
        x[i + 3*quarter] = x[i + 2*quarter];
        y[i + 3*quarter] = y[i + 2*quarter] - offset - 1;
        // copy Top-Left-Moore -> Bottom-Right-Moore
        x[i] = x[i + quarter];
        y[i] = y[i + quarter] - offset - 1;
    }
}
