.intel_syntax noprefix

.global moore
.global _moore // extern function name must start with underscore if compiled on mac

.text


moore:
_moore: // mac compatibility
    push rbx
    push r12
    push r13
    push r14
    push r15
    push rsi
    push rdx

    // Args
    //
    // edi:     degree
    // rsi:     *x
    // rdx:     *y

    // Registers
    //
    // CL:      shifts
    // r15:     hilbert_loop_counter
    // r14:     number of coordinates in previous hilbert iteration
    // r13:     side length of previous hilbert iteration
    // r12:     hilbert_inner_loop_counter
    // r11,r10: tmp registers
    //
    // xmm13:       helper register (holds 4 copies of (side length - 1) of previous hilbert iteration)
    // xmm14:       helper register (holds 4 copies of side length of previous hilbert iteration)
    // xmm15:       helper register (holds 4 copies of `1`)
    // xmm0:        holds current x vector
    // xmm1:        holds current y vector
    // xmm2, xmm3:  x and y values in quadrant B
    // xmm4, xmm5:  x and y values in quadrant C
    // xmm6, xmm7:  x and y values in quadrant D
    // xmm8:        temp register

    // write coordinates of first iteration of hilbert curve
    // into the array
    mov DWORD PTR [rsi +  0], 0x00
    mov DWORD PTR [rsi +  4], 0x00
    mov DWORD PTR [rsi +  8], 0x01
    mov DWORD PTR [rsi + 12], 0x01

    mov DWORD PTR [rdx +  0], 0x00
    mov DWORD PTR [rdx +  4], 0x01
    mov DWORD PTR [rdx +  8], 0x01
    mov DWORD PTR [rdx + 12], 0x00

    // if we want to know moore(1) we are done
    cmp edi, 0x01
    jle .L_done

    // we need hilbert(degree-1) to calculate moore(degree)
    // initialize counter with c = degree - 2
    xor r15, r15
    mov r15d, edi
    sub r15d, 0x02

    // nr of coordinates in hilbert(1) is 4
    mov r14, 0x04
    mov r13, 0x02

    // init temp register that holds 4 copies of 1
    mov r11d, 0x01                  // use temp register
    movd xmm15, r11d
    pshufd xmm15, xmm15, 0x00

    // init temp register that holds previous side length
    movd xmm14, r14d
    pshufd xmm14, xmm14, 0x00
    movdqa xmm13, xmm14
    psubd xmm13, xmm15

.L_hilbert_loop_body:
    xor r12, r12                    // hilbert_inner_counter with 0

.L_hilbert_inner_loop_body:         // iterate over all coordinates from previous hilbert iteration
    movdqu xmm0, [rsi + r12]        // fill xmm registers with next 4 coordinates we want to process
    movdqu xmm1, [rdx + r12]

    // copy coordinates from A -> B
    movdqa xmm2, xmm0               // xmm2 holds x values that go into quadrant B
    movdqa xmm3, xmm1               // xmm3 holds y values that go into quadrant B
    paddd xmm3, xmm14               // shift y values upwards (into quadrant B)

    // copy coordinates from B -> C
    movdqa xmm4, xmm2               // xmm4 holds x values that go into quadrant C
    movdqa xmm5, xmm3               // xmm5 holds y values that go into quadrant C
    paddd xmm4, xmm14               // shift x values right (into quadrant C)

    // swap(x, y) in quadrant A
    movdqa xmm8, xmm0
    movdqa xmm0, xmm1
    movdqa xmm1, xmm8

    // copy and transform A -> D
    // x -> 2 * len - 1 - x
    // y -> len - 1 - y
    movdqa xmm6, xmm14              // xmm6 holds x values that go into quadrant D
    paddd xmm6, xmm13
    psubd xmm6, xmm0

    movdqa xmm7, xmm13              // xmm6 holds y values that go into quadrant D
    psubd xmm7, xmm1

    // write register content to memory
    // init temp registers
    mov r10, rsi                        // r10 = rsi + r12
    add r10, r12
    mov r11, rdx                        // r11 = rdx + r12
    add r11, r12

    movdqu [r10], xmm0      // quadrant A
    movdqu [r11], xmm1

    add r10, r14
    add r11, r14
    movdqu [r10], xmm2     // quadrant B
    movdqu [r11], xmm3

    add r10, r14
    add r11, r14
    movdqu [r10], xmm4    // quadrant C
    movdqu [r11], xmm5

    add r10, r14
    add r11, r14
    movdqu [r10], xmm6    // quadrant D
    movdqu [r11], xmm7

    add r12, 0x04                   // increase counter by 4 since we processed 4 points in one step

.L_hilbert_inner_loop_cond:
    cmp r12, r14
    jnz .L_hilbert_inner_loop_body

    shl r14, 0x02                   // number of points *= 4
    shl r13, 0x01                   // side length *= 2

    pslld xmm14, 0x02
    movdqa xmm13, xmm14
    psubd xmm13, xmm15

    sub r15, 0x01                   // decrease counter by one

.L_hilbert_loop_cond:
    jnz .L_hilbert_loop_body        // flags are still set from decreasing counter

    // moore stuff



.L_done:
    pop rdx
    pop rsi
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx

    ret