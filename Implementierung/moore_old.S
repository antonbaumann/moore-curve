.intel_syntax noprefix

.global moore
.global _moore // extern function name must start with underscore if compiled on mac

.text


moore:
_moore: // mac compatibility

/*
Args (when incoming):
    degree: in rdi
    x: in rsi
    y: in rdx

Registers (during loop):

    r8: iterations = quarter of points = 2^(degree*2-2)
    rcx: used for shifting
    rdi: degree
    r10: counter & index of current point
    r12, r13: temp registers
    rsi: current x address
    rdx: current y address
    rax: current x-coord value
    rbx: current y-coord value
    r14: half of sidelength (rounded down)
    r15: half of sidelength (rounded up)


==============INITIALISE VALUES AND PUSH CALLEE-SAVED REGISTERS==========
*/
 
    push rbx  	  
    push r12
    push r13
    push r14
    push r15
    //save start adresses for later
    push rsi
    push rdx

    mov rcx, rdi
    sub rcx, 1
    mov r14, 1
    shl r14, cl    // r14 = 2^(degree-1) = half of sidelength
    mov r15, r14
    dec r14	   // (rounded down)
    shl rcx, 1     // rcx = 2*degree - 2
    mov r8, 1
    shl r8, cl     // r8 := 2 ^ (2 * degree-2) = quarter of points

    mov r10, 0     // counter := 0

//=========BEGIN BIG LOOP OVER QUARTER OF POINTS============

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
      rax: x-coord
      rbx: y-coord
    */
  

    xor rax, rax
    xor rbx, rbx
    mov r11, r10     // create copy of index
    mov r9, 1        // initialize counter

//------ BEGIN LOOP FOR POINT POSITIONING (ROTATE & TRANSLATE-------

.LHilbert_body:
    /* rotate
    Uses:
      r11: as modifiable index (source for bits that are relevant for positioning)     
    */   
    test r11, 0b10      	// look at second to last bit
    jz .LHilbert_left   	// if zero, then we are in the left half,
.LHilbert_right:        	// else we are right
    test r11, 1			// look at last bit
    jz .LHilbert_continue 	// if zero, no rotation necessary
    mov r13, r9         	// else mirroring like described below	
    dec r13			// r13 := current sidelength - 1
    mov r12, rax        	// save x value in tmp register
    mov rax, r13
    sub rax, rbx        	// x' := length - 1 - y
    mov rbx, r13
    sub rbx, r12        	// y' := length - 1 - x
    jmp .LHilbert_continue
.LHilbert_left:
    test r11, 1         	// we are in left half, look at last bit
    jnz .LHilbert_continue	// if last bit is 1, no rotation is necessary
    mov r12, rax 		// else, swap rax and rbx
    mov rax, rbx
    mov rbx, r12

//ROTATION DONE

//BEGIN TRANSLATION

.LHilbert_continue:
    test r11, 0b10		// Like above, if second to last bit is set we are in right half and need to translate
    jz .LHilbert_x_Translation_done
    add rax, r9			// if right -> translate to right by current sidelength
.LHilbert_x_Translation_done:
    mov r12, r11		// Here we compare the two last bits. If they are the same...
    shr r12, 1
    xor r12, r11
    test r12, 1
    jz .LHilbert_y_Translation_done	// ...then we dont need to translate vertically
    add rbx, r9				// else translate upwards by current sidelength
.LHilbert_y_Translation_done:
    shr r11, 2       // throw away last two bits of index to look at next two bits
    shl r9, 1        // counter *= 2, sidelength *= 2

//TRANSLATION DONE

.LHilbert_cond:
    cmp r9, r8      // counter - max_iterations
    jle .LHilbert_body

//--------END LOOP FOR POINT POSITIONING--------

    // if hilbert degree is odd -> swap x and y
    test rdi, 1    
    jz .LHilbert_even
    mov r12, rax        // swap rax and rbx
    mov rax, rbx
    mov rbx, r12
.LHilbert_even:

    // rotate counterclockwise to create lower left moore quarter
    // (x, y) -> (half_sidelength - y, x)
    mov r12, rax
    mov rax, r14
    sub rax, rbx
    mov rbx, r12

    // Write coords into memory
    mov [rsi], rax
    mov [rdx], rbx
    // Increase y-pointer, x-pointer and loopcounter
    add rsi, 8
    add rdx, 8
    inc r10
.Lcond:
    cmp r10, r8     // counter - max_iterations
    jl .Lstart
 
//==========END BIG LOOP OVER QUARTER OF POINTS============

//==========BEGIN COPYING AND ROTATION/TRANSLATION OF CALCULATED QUARTER======
//TODO: Parallelise more with xmm registers?

    // copy from lower left upper left, translate by half sidelength
    xor r10, r10 	// reset counter
    pop r12		// y beginning adress in r12
    pop r13		// x beginning adress in r13

.LMoore_body_left_upper:
    //Load corresponding hilbert-points
    mov rax, [r13]
    mov rbx, [r12]
    // manipulate: (x, y) -> (x, sidelength+y)
    add rbx, r15
    //Write coords into memory
    mov [rsi], rax
    mov [rdx], rbx
    // Increase y-pointer, x-pointer and loopcounter
    add r12, 8
    add r13, 8
    add rsi, 8
    add rdx, 8
    inc r10
    cmp r10, r8
    jl .LMoore_body_left_upper
    

    //mirror from upper left to upper right
    xor r10, r10 	//reset loopcounter
.LMoore_body_right_upper:
    //Load corresponding hilbert-points
    mov rax, [r13]
    mov rbx, [r12]
    // manipulate: (x, y) -> (2*sidelength-x-1, 3*sidelength-y-1))
    neg rax
    add rax, r15
    add rax, r14
    neg rbx
    add rbx, r15
    add rbx, r15
    add rbx, r14
    //Write coords into memory
    mov [rsi], rax
    mov [rdx], rbx
    // Increase y-pointer, x-pointer and loopcounter
    add r12, 8
    add r13, 8
    add rsi, 8
    add rdx, 8
    inc r10
    cmp r10, r8
    jl .LMoore_body_right_upper

    
    // copy from upper right and translate down to lower right
    xor r10, r10	// reset counter    
.LMoore_body_right_lower:
    //Load corresponding hilbert-points
    mov rax, [r13]
    mov rbx, [r12]
    // manipulate: (x, y) -> (x, y-sidelength)
    sub rbx, r15
    //Write coords into memory
    mov [rsi], rax
    mov [rdx], rbx
    // Increase y-pointer, x-pointer and loopcounter
    add r12, 8
    add r13, 8
    add rsi, 8
    add rdx, 8
    inc r10
    cmp r10, r8
    jl .LMoore_body_right_lower

//===== restore registers =====
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret
