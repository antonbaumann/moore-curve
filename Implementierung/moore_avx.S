.intel_syntax noprefix

.global moore_avx
.global _moore_avx // extern function name must start with underscore if compiled on mac

.text


moore_avx:
_moore_avx: // mac compatibility
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

    // nr of coordinates in hilbert(1) is 4
    mov r14, 0x04
    mov r13, 0x02      // side len

    // init temp register that holds 4 copies of 1
    mov r11d, 0x01                  // use temp register
    movd xmm15, r11d
    pshufd xmm15, xmm15, 0x00

    // init temp register that holds previous side length
    movd xmm14, r13d
    pshufd xmm14, xmm14, 0x00
    movdqa xmm13, xmm14
    psubd xmm13, xmm15

    // if we want to know moore(1) we are done
    cmp edi, 0x01
    jle .L_done

    // id degree = 2 -> generate moore
    cmp edi, 0x02
    je .L_moore_transformation_128

    // if we are here: degree >= 3
    // so we can use 256 bit registers
    VBROADCASTSS ymm15, xmm15
    VBROADCASTSS ymm14, xmm14
    VBROADCASTSS ymm13, xmm13

    // we need hilbert(degree-1) to calculate moore(degree)
    // initialize counter with c = degree - 2
    xor r15, r15
    mov r15d, edi
    sub r15d, 0x02

.L_hilbert_loop_body:
    xor r12, r12                    // hilbert_inner_counter with 0

.L_hilbert_inner_loop_body:         // iterate over all coordinates from previous hilbert iteration

    // write register content to memory
    // init temp registers
    lea r10, [rsi + 4* r12]         // r10 = rsi + 4 * r12
    lea r11, [rdx + 4* r12]         // r11 = rdx + 4 * r12

    movdqu xmm0, [r10]              // fill xmm registers with next 4 coordinates we want to process
    movdqu xmm1, [r11]

    // copy coordinates from A -> B
    // xmm0 holds correct x values for quardrant B
    movdqa xmm3, xmm1               // xmm3 holds y values that go into quadrant B
    paddd xmm3, xmm14               // shift y values upwards (into quadrant B)

    // copy coordinates from B -> C
    // xmm3 holds correct y vaues for quadrant C
    movdqa xmm4, xmm0               // xmm4 holds x values that go into quadrant C
    paddd xmm4, xmm14               // shift x values right (into quadrant C)

    // copy and transform A -> D
    // x -> 2 * len - 1 - x
    // y -> len - 1 - y
    movdqa xmm6, xmm14              // xmm6 holds x values that go into quadrant D
    paddd xmm6, xmm13
    psubd xmm6, xmm1
    movdqa xmm7, xmm13              // xmm6 holds y values that go into quadrant D
    psubd xmm7, xmm0

    movdqu [r10], xmm1              // quadrant A
    movdqu [r11], xmm0              // swap x and y values

    lea r10, [r10 + 4*r14]
    lea r11, [r11 + 4*r14]

    movdqu [r10], xmm0              // quadrant B
    movdqu [r11], xmm3

    movdqu [r10 + 4 * r14], xmm4    // quadrant C
    movdqu [r11 + 4 * r14], xmm3

    movdqu [r10 + 8 * r14], xmm6    // quadrant D
    movdqu [r11 + 8 * r14], xmm7

    add r12, 0x04                   // increase counter by 4 since we processed 4 points in one step

.L_hilbert_inner_loop_cond:
    cmp r12, r14
    jnz .L_hilbert_inner_loop_body

    shl r14, 0x02                   // number of points *= 4

    pslld xmm14, 0x01               // side len *= 2
    movdqa xmm13, xmm14
    psubd xmm13, xmm15              // xmm13 = xmm14 - 1

    sub r15, 0x01                   // decrease counter by one

.L_hilbert_loop_cond:
    jnz .L_hilbert_loop_body        // flags are still set from decreasing counter

.L_moore_transformation_128:
    xor r15, r15                    // set counter to 0

.L_moore_loop_body_128:
    // write register content to memory
    // init temp registers
    lea r10, [rsi + 4* r15]         // r10 = rsi + 4 * r12
    lea r11, [rdx + 4* r15]         // r11 = rdx + 4 * r12

    movdqu xmm0, [r10]
    movdqu xmm1, [r11]

    // copy coordinates from bottom left to top left (hilbert A -> moore B)
    movdqa xmm2, xmm13              // x -> len - 1 - y
    psubd xmm2, xmm1
    movdqa xmm3, xmm14              // y -> len + x
    paddd xmm3, xmm0

    // copy (hilbert A -> moore D)
    movdqa xmm6, xmm1               // x -> y + len
    paddd xmm6, xmm14
    movdqa xmm7, xmm13              // y -> len - 1 - x
    psubd xmm7, xmm0

    // copy (moore D -> moore C)
    movdqa xmm5, xmm7               // y -> y + len
    paddd xmm5, xmm14               // correct x values already in xmm6

    // copy (moore B -> moore A)
    movdqa xmm1, xmm3               // y -> y - len
    psubd xmm1, xmm14               // correct x values already in xmm2

    movdqu [r10], xmm2              // quadrant A
    movdqu [r11], xmm1              // use x values from quadrant B

    lea r10, [r10 + 4 * r14]
    lea r11, [r11 + 4 * r14]

    movdqu [r10], xmm2              // quadrant B
    movdqu [r11], xmm3

    movdqu [r10 + 4*r14], xmm6      // quadrant C
    movdqu [r11 + 4*r14], xmm5      // use x values from quadrant D

    movdqu [r10 + 8*r14], xmm6      // quadrant D
    movdqu [r11 + 8*r14], xmm7

    add r15, 0x04

.L_moore_loop_cond_128:
    cmp r15, r14
    jl .L_moore_loop_body_128

.L_done:
    pop rdx
    pop rsi
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx

    ret