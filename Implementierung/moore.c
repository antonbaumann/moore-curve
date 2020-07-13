#include <stdint.h>

// makes returning two variables more comfortable
struct tuple {
    uint64_t x, y;
};

// rotates/modifies shape for all 4 quadrants
struct tuple rotate(struct tuple coord, uint32_t length, uint32_t top, uint32_t right) {
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
        uint64_t right = (uint64_t) 1 & (index /2);    // to decide what Quadrant look up the 2 least significant Bits of i, for ex. 7 = 0b01(11)
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
        return hilbert_coord_at_index(index, 1);
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

void moore_c_iterative(uint32_t degree, uint32_t *x, uint32_t *y) {
    uint64_t shifts = 2 * degree - 1;
    uint64_t max_iterations = (uint64_t) 2
            << shifts; // 2 ^ (2n) , each shift == 2* in binary we use this to iterate over the index in binary for ex. 7 == 0b0111
    // pass max_iterations which equals amount of points 2^(2n) = 4^n
    for (uint64_t i = 0; i < max_iterations; i++) {
        struct tuple coord = moore_coord_at_index(i, degree, max_iterations);
        x[i] = coord.x;
        y[i] = coord.y;
    }
}

void hilbert_c_iterative(uint32_t degree, uint32_t *x, uint32_t *y) {
    uint64_t shifts = 2 * degree - 1;
    uint64_t max_iterations = (uint64_t) 2
            << shifts; // 2 ^ (2n) , each shift == 2* in binary we use this to iterate over the index in binary for ex. 7 == 0b0111
    // pass max_iterations which equals amount of points 2^(2n) = 4^n

    for (uint64_t i = 0; i < max_iterations; i++) {
        struct tuple coord = hilbert_coord_at_index(i, degree);
        x[i] = coord.x;
        y[i] = coord.y;
    }
}

//void s_to_b(uint64_t total, uint64_t side, uint32_t *x, uint32_t *y){
//    uint64_t *new_x = x + total;
//    uint64_t *new_y = y + total;
//    for (uint64_t i = 0; i < total; i++){
//        new_y[i] = side + y[i];
//        new_x[i] = x[i];
//    }
//}
//
//void b_to_a(uint64_t total, uint64_t *x, uint64_t *y){
//    uint64_t temp = 0;
//    for (uint64_t i = 0; i < total; i++){
//        temp = x[i];
//        x[i] = y[i];
//        y[i] = temp;
//    }
//}
//
//void b_to_c(uint64_t total, uint64_t side, uint64_t *x, uint64_t *y){
//    uint64_t *new_x = x + 2*total;
//    uint64_t *new_y = y + 2*total;
//    for (uint64_t i = 0; i < total; i++){
//        new_x[i] = x[i + total] + side;
//        new_y[i] = y[i + total];
//    }
//}
//
//void a_to_d(uint64_t total, uint64_t side, uint64_t *x, uint64_t *y){
//    uint64_t *new_x = x + 3*total;
//    uint64_t *new_y = y + 3*total;
//    for (uint64_t i = 0; i < total; i++){
//        new_x[i] = (side - 1) - x[i] + side;
//        new_y[i] = (side - 1) - y[i];
//    }
//}

void hilbert_c_batch(uint32_t degree, uint64_t *x, uint64_t *y) {
//    x[0] = 0; y[0] = 0;
//    x[1] = 0; y[1] = 1;
//    x[2] = 1; y[2] = 1;
//    x[3] = 1; y[3] = 0;
//    if (degree == 1){
//        return;
//    }
//    uint64_t total = 4;
//    uint64_t side = 2;
//    for (uint64_t i = 2; i <= degree; i++){
//        s_to_b(total, side, x, y);
//        b_to_c(total, side, x, y);
//        b_to_a(total, x, y);
//        a_to_d(total, side, x, y);
//        total *= 4;
//        side *= 2;
//    }
}

void moore_c_batch(uint32_t degree, uint64_t *x, uint64_t *y) {
//    if (degree == 1) {
//        return moore_c_iterative(degree, x, y);
//    }
//
//    hilbert_c_batch(degree - 1, x, y);
//
//    uint64_t quarter = (uint64_t) 2 << (2 * degree - 3); // times 4 equals amount of all points
//
//    // half the sidelength of the square with our moore curve (rounded down)  -> amount of x/y translation
//    uint64_t offset = ((uint64_t) 2 << (degree - 2)) - 1;
//
//    //rotate to the left and translate up to create upper left quarter of moore; x -> d-y, y -> d+x+1
//    for (uint64_t i = quarter; i < 2 * quarter; i++) {
//        x[i] = offset - y[i - quarter];
//        y[i] = x[i - quarter] + offset + 1;
//    }
//
//    // draw top right quadrant; x -> d+y+1, y -> 2d-x+1 (from hilbert curve)
//    for (uint64_t i = 2 * quarter; i < 3 * quarter; i++) {
//        x[i] = y[i - 2 * quarter] + offset + 1;
//        y[i] = 2 * offset - x[i - 2 * quarter] + 1;
//    }
//
//    //draw bottom right quadrant (translate from above)
//    for (uint64_t i = 3 * quarter; i < 4 * quarter; i++) {
//        x[i] = x[i - quarter];
//        y[i] = y[i - quarter] - offset - 1;
//    }
//
//    //now redraw bottom left quadrant, copy from above quadrant
//    for (uint64_t i = 0; i < quarter; i++) {
//        x[i] = x[i + quarter];
//        y[i] = y[i + quarter] - offset - 1;
//    }
}