.intel_syntax noprefix
.global moore

.text

// void moore( uint64_t degree, uint64_t *x, uint64_t *y)
moore:
/*
Args (when incoming):
    degree: in rdi
    x: in rsi
    y: in rdx

Registers (during loop):

    r8: max iterations
    rcx: degree * 2
    rdi: degree
    r10: counter & index of current point
    rsi: current x address
    rdx: current y address
    rax: current x-coord value
    rbx: current y-coord value
*/
    push rbx  	   // save rbx
    push r12
    push r13
    push r14
    push r15

    mov rcx, rdi
    shl rcx, 1     // degree *= 2

    mov r8, 1
    shl r8, cl     // r8 := 2 ^ (2 * degree)

    mov r10, 0     // counter := 1

    //———1. Iterate over all points
    jmp .Lcond 	   // maybe unnecessary
.Lstart:
    /* calculate hilbert coordinates at index
    Expecting:
      r10: index
      rdi: degree
      r8:  max_iterations
    Uses:
      r9:  counter
      r11: index_copy
    Returns:
      rax: x-ccord
      rbx: y-coord
    */

    xor rax, rax
    xor rbx, rbx
    mov r11, r10     // create copy of index
    mov r9, 1        // initialize counter

.LHilbert_body:
    /* rotate
    Uses:
      r11: as modifiable index
    
    */   
    test r11, 0b10   // look at second to last bit
    jz .LHilbert_left
.LHilbert_right:
    test r11, 1		// look at last bit
    jz .LHilbert_continue
    mov r13, r9         
    dec r13		// r13 := length - 1
    mov r12, rax        // save x value in tmp register
    mov rax, r13
    sub rax, rbx        // x' := length - 1 - y
    mov rbx, r13
    sub rbx, r12        // y' := length - 1 - x
    jmp .LHilbert_continue
.LHilbert_left:
    test r11, 1         // look at last bit
    jnz .LHilbert_continue
    mov r12, rax 	// swap rax and rbx
    mov rax, rbx
    mov rbx, r12
.LHilbert_continue:
    test r11, 0b10
    jz .LHilbert_skip_1
    add rax, r9		// if right -> translate to right by current len
.LHilbert_skip_1:
    mov r12, r11
    shr r12, 1
    xor r12, r11
    test r12, 1
    jz .LHilbert_skip_2
    add rbx, r9         // if top -> translate to top by current len
.LHilbert_skip_2:
    shr r11, 2       // throw away last two bits of index
    shl r9, 1        // counter *= 2

.LHilbert_cond:
    cmp r9, r8      // counter - max_iterations
    jle .LHilbert_body

    // if hilbert degree is odd -> swap x and y
    test rdi, 1    
    jnz .LHilbert_even
    mov r12, rax        // swap rax and rbx
    mov rax, rbx
    mov rbx, r12
.LHilbert_even:    
    // todo: sog. moore translation

    mov [rsi], rax
    mov [rdx], rbx

    add rsi, 8
    add rdx, 8
    inc r10
.Lcond:
    cmp r10, r8     // counter - max_iterations
    jle .Lstart
    //———

    pop r15
    pop r14
    pop r13
    pop r12

    pop rbx
    ret
