#include <stdio.h>
#include <stdint.h>

// makes returning two variables more comfortable
struct tuple {
    uint64_t x, y;
};


struct tuple rot(
        uint64_t x,
        uint64_t y,
        uint64_t rx,
        uint64_t ry,
        uint64_t p
) {
    if (ry == 0) {
        if (rx == 1) {
            x = p - 1 - x;
            y = p - 1 - y;
        }
        // vertausche x und y
        uint64_t z = x;
        x = y;
        y = z;
    }
    struct tuple t = {.x = x, .y = y};
    return t;
}

struct tuple m(uint64_t t, uint64_t p) {
    uint64_t x = 0;
    uint64_t y = 0;

    for (uint64_t m = 1; m < p; m *= 2) {
        uint64_t rx = (uint64_t) 1 & (t / 2);
        uint64_t ry = (uint64_t) 1 & (t ^ rx);
        struct tuple pair = rot(x, y, rx, ry, m);
        x = pair.x;
        y = pair.y;
        x += m * rx;
        y += m * ry;
        t /= 4; // zur nächsten Quaternärziffer
    }

    struct tuple pair = {.x = x, .y = y};
    return pair;
}

void hilbert_c_iterative(long degree, uint64_t *x_coords, uint64_t *y_coords) {

    unsigned int shifts = 2 * degree - 1;
    unsigned long long nr_iterations = (unsigned long long) 2 << shifts; // 2 ^ (2 * degree)

    for (uint64_t i = 0; i < nr_iterations; i++) {
        if (i % 10000 == 0) {
            printf("%llu / %llu\r", i, nr_iterations);
        }
        struct tuple pair = m(i, nr_iterations);
        x_coords[i] = pair.x;
        y_coords[i] = pair.y;
    }

    //The algorithm returns curves of even/ uneven degree differently. To normalize this, we mirror, rotate and translate the result of 
    // uneven uneven hilbert-degrees (= even moore degrees)
    if (degree%2 !=0) {

    	//First, start and endpoint have to be switched (= reverse order of traversal)
	for (uint64_t i = 0; i < nr_iterations/2; i++) {
		uint64_t temp = x_coords[i];
		x_coords[i] = x_coords[nr_iterations-1-i];
		x_coords[nr_iterations-1-i] = temp;
		temp = y_coords[i];		
                y_coords[i] = y_coords[nr_iterations-1-i];
		y_coords[nr_iterations-1-i] = temp;
     	}

	unsigned int d = (2 << (degree - 1)) - 1; //half the sidelength of the square with our moore curve (rounded down)  -> amount of x/y translation
	//second, rotate counterclockwise by 90 degrees x-> d-y, y -> x
        for (uint64_t i = 0; i < nr_iterations; i++) {
		uint64_t temp = x_coords[i];
 		x_coords[i] = d-y_coords[i];
		y_coords[i] = temp;
        }

    }
    


}

int moore_c_iterative(long degree, uint64_t *x_coords, uint64_t *y_coords) {
    
    printf("moore c iterative: degree %ld\n", degree);

    //Moore and Hilbert are the same for n = 1
    if (degree == 1) {
        hilbert_c_iterative(1, x_coords, y_coords);
        return 0;
    }

    //Build hilbert curve for lower left quarter (hence degree -1)
    hilbert_c_iterative(degree-1, x_coords, y_coords);
     unsigned long long quarter = 2 << (2*degree-3); // times 4 equals amount of all points

    unsigned int d = (2 << (degree - 2)) - 1; //half the sidelength of the square with our moore curve (rounded down)  -> amount of x/y translation
    unsigned long long counter = quarter; // quarter of the points are already drawn, wrong for now but we start with upper left quadrant

    //rotate to the left and translate up to create upper left quarter of moore; x -> d-y, y -> d+x+1
    while (counter < 2*quarter) {
        y_coords[counter] = x_coords[counter - quarter] + d + 1;
        x_coords[counter] = d - y_coords[counter - quarter];
        counter++;
    }
    // draw top right quadrant; x -> d+y+1, y -> 2d-x+1 (from hilbert curve)
    while (counter < 3*quarter) {
        x_coords[counter] = y_coords[counter -2*quarter] + d + 1;
        y_coords[counter] = 2*d - x_coords[counter -2*quarter] + 1;
        counter++;
    }
    //draw bottom right quadrant (translate from above)
    while (counter < 4*quarter) {
        x_coords[counter] = x_coords[counter - quarter];
        y_coords[counter] = y_coords[counter - quarter] - d - 1;
        counter++;
    }
    //now redraw bottom left quadrant, copy from above quadrant
    counter = 0;
    while (counter < quarter) {
        x_coords[counter] = x_coords[counter + quarter];
        y_coords[counter] = y_coords[counter + quarter] - d - 1;
        counter++;
    }

    return 0;
}
