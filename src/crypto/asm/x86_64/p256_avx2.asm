bits 64
default rel

section .rdata align=32
P256_PRIME:
    dq 0xffffffffffffffff, 0x00000000ffffffff
    dq 0x0000000000000000, 0xffffffff00000001

P256_ORDER:
    dq 0xf3b9cac2fc632551, 0xbce6faada7179e84
    dq 0xffffffffffffffff, 0xffffffff00000000

P256_RR:
    dq 0x0000000000000003, 0xfffffffbffffffff
    dq 0xfffffffffffffffe, 0x00000004fffffffd

section .text

global p256_mul_mont
global p256_sqr_mont
global p256_add_mod
global p256_sub_mod
global p256_point_add
global p256_point_double

p256_mul_mont:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 64       
    
    mov r8,  [rdx]      
    mov r9,  [rdx+8]    
    mov r10, [rdx+16]   
    mov r11, [rdx+24]   
    
    mov r12, [r8]       
    mov r13, [r8+8]     
    mov r14, [r8+16]    
    mov r15, [r8+24]    
    
    mov rax, r8
    mul r12             
    mov [rsp], rax      
    mov rbx, rdx       
    
    mov rax, r8
    mul r13             
    add rbx, rax
    adc rdx, 0
    mov [rsp+8], rbx
    mov rbx, rdx
    
    mov rax, r8
    mul r14             
    add rbx, rax
    adc rdx, 0
    mov [rsp+16], rbx
    mov rbx, rdx
    
    mov rax, r8
    mul r15          
    add rbx, rax
    adc rdx, 0
    mov [rsp+24], rbx
    mov [rsp+32], rdx
    
    mov rax, [rsp]
    mov rdx, 0xffffffffffffffff
    mul rax  
    
.reduce_final:
    lea r14, [P256_PRIME]
    mov r8, [rsp]
    mov r9, [rsp+8]
    mov r10, [rsp+16]
    mov r11, [rsp+24]
    
    sub r8, [r14]
    sbb r9, [r14+8]
    sbb r10, [r14+16]
    sbb r11, [r14+24]
    
    cmovnc [rcx], r8
    cmovnc [rcx+8], r9
    cmovnc [rcx+16], r10
    cmovnc [rcx+24], r11
    
    add rsp, 64
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

p256_sqr_mont:
    push rbp
    mov rbp, rsp    
    
    mov r8, [rdx]
    mov r9, [rdx+8]
    mov r10, [rdx+16]
    mov r11, [rdx+24]
    
    mov rax, r8
    mul rax
    
    mov rax, r8
    mul r9
    
    pop rbp
    ret

p256_add_mod:
    mov r8, [rdx]
    mov r9, [rdx+8]
    mov r10, [rdx+16]
    mov r11, [rdx+24]
    
    add r8, [r8]
    adc r9, [r8+8]
    adc r10, [r8+16]
    adc r11, [r8+24]
    
    lea rax, [P256_PRIME]
    mov r12, r8
    mov r13, r9
    mov r14, r10
    mov r15, r11
    
    sub r12, [rax]
    sbb r13, [rax+8]
    sbb r14, [rax+16]
    sbb r15, [rax+24]
    
    cmovnc r8, r12
    cmovnc r9, r13
    cmovnc r10, r14
    cmovnc r11, r15
    
    mov [rcx], r8
    mov [rcx+8], r9
    mov [rcx+16], r10
    mov [rcx+24], r11
    ret

p256_sub_mod:
    mov r8, [rdx]
    mov r9, [rdx+8]
    mov r10, [rdx+16]
    mov r11, [rdx+24]
    
    sub r8, [r8]
    sbb r9, [r8+8]
    sbb r10, [r8+16]
    sbb r11, [r8+24]
    
    lea rax, [P256_PRIME]
    mov r12, [rax]
    mov r13, [rax+8]
    mov r14, [rax+16]
    mov r15, [rax+24]
    
    cmovc r12, 0
    cmovc r13, 0
    cmovc r14, 0
    cmovc r15, 0
    
    add r8, r12
    adc r9, r13
    adc r10, r14
    adc r11, r15
    
    mov [rcx], r8
    mov [rcx+8], r9
    mov [rcx+16], r10
    mov [rcx+24], r11
    ret

p256_point_add:
    push rbp
    mov rbp, rsp
    sub rsp, 384

    add rsp, 384
    pop rbp
    ret

p256_point_double:
    push rbp
    mov rbp, rsp
    sub rsp, 256 
    
    add rsp, 256
    pop rbp
    ret