SECTION .data

instruction db "please input two numbers:", 0h
str1 db "sum:", 0h
str2 db "product:", 0h
error db "Invalid", 0h


SECTION .bss

numbers: resb 255
num1: resb 255
num2: resb 255
sum: resb 255
product: resb 255
operator: resb 1

SECTION .text

global _start

; ================================================
; Function: clear all  the variables and registers
clear:                    
    mov eax, 0            ; clear the registers
    mov ebx, 0
    mov ecx, 0
    mov edx, 0

    mov eax, numbers ; the first address
    mov cx,1282           ; size
L1:
    mov BYTE[eax], 0          
    inc eax
    loop L1

    mov eax, 0            ; clear the registers
    mov ebx, 0
    mov ecx, 0
    mov edx, 0
    ret
    

; ========================
; Function: print a string
printstr:                
    push edx         
    push ecx
    push ebx
    push eax

    xor ecx, ecx
    mov ecx, eax
    call length      ; get length of param
    mov edx, eax
    mov ebx, 1
    mov eax, 4
    int 80h

    pop eax          
    pop ebx
    pop ecx
    pop edx
    ret


; ===========================
; Function: print a character
put:
    push edx
    push ecx
    push ebx
    push eax

    mov eax, 4
    mov ebx, 1
    mov ecx, esp
    mov edx, 1
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx
    ret


; ====================
; Function: print line
println:
    push eax
    mov eax,0Ah
    call put
    pop eax
    ret


; ========================================
; Function: get the length of ouput string
length:              
    push ebx

    xor ebx, ebx     
    mov ebx, eax
.next:
    cmp BYTE[ebx], 0 ; whether it equals '\0'
    jz .fin          ; jump to .fin if equal
    inc ebx          
    jmp .next        ; loop
.fin:
    sub ebx, eax
    xor eax, eax
    mov eax, ebx

    pop ebx
    ret


; ====================================
; Function: read a line from std input
readline:            
    push edx
    push ecx
    push ebx
    push eax

    mov edx, ebx
    mov ecx, eax
    mov ebx, 1
    mov eax, 3
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx

    ret


; =======================
; Function: sub one digit
sub_1:                 
    mov ecx, 1
    sub al, 10
    ret

parse_input:               ; Function
.loop:
    ;change2
    cmp BYTE[ecx], 10      ; if the char is ENTER
    jz .end
    cmp BYTE[ecx], 43      ; if the char is '+'
    jz .save
    cmp BYTE[ecx], 42      ; if the char is '*'
    jz .save
    cmp BYTE[ecx], 42      ; if the char is invalid
    jl .error
    cmp BYTE[ecx], 57      ; if the char is invalid
    jg .error
    mov dl, BYTE[ecx]
    mov BYTE[eax], dl
    inc eax
    inc ecx
    jmp .loop

.save:
    mov ebx, operator
    mov esi, ebx
    mov dl, BYTE[ecx]
    mov BYTE[esi], dl
    inc ecx
    ret

.error:
    mov eax, error
    call printstr
    call println
    int 80h
    
    jmp _start

.end:
    inc ecx
    ret


; ================
; Function: do mul
inner_loopMul:                  
    push esi
    push ebx
    push edx

.loop:
    cmp esi, num1       ; mul by digits
    jl .fin
    xor eax, eax
    xor ebx, ebx
    add al, BYTE[esi]
    sub al, 48
    add bl, BYTE[edi]           ; add by digits
    sub bl, 48
    mul bl
    add BYTE[edx], al
    mov al, BYTE[edx]
    mov ah, 0
    mov bl, 10
    div bl
    mov BYTE[edx], ah
    dec esi                     ; move ptr1
    dec edx                     ; move result ptr
    add BYTE[edx], al
    jmp .loop

.fin:
    mov eax, edx
    pop edx
    pop ebx
    pop esi
    ret


format:                         ;Function
    push edx
    push eax
.loop:
    mov eax, product
    add eax, 255
    cmp edx, eax                ;need to fomat
    jge .fin
    add BYTE[edx], 48
    inc edx
    jmp .loop
.fin:
    pop eax
    pop edx 
    ret


_start:                        ; start
.loopstart:
    call clear                 ; do clear job

    ;change1

    mov eax, numbers
    mov ebx, 255
    call readline

    ; stop the program when input 'q'
    mov esi, numbers
    cmp BYTE[esi], 113
    jz .endstart

    mov ecx, numbers

    mov eax, num1
    call parse_input

    mov eax, num2
    call parse_input

    mov ecx, operator
    cmp BYTE[ecx], 42
    jz .after_input
    cmp BYTE[ecx], 43
    jz .after_input

    
    jmp .loopstart

.after_input:
    mov ecx, 0
    mov edx, sum
    add edx, 255
    xor eax, eax
    mov al, 10
    mov BYTE[edx], al

    mov eax, num1              ; get len of the first number
    call length                ; esi: ptr of num1
    mov esi, eax
    add esi, num1
    sub esi, 1

    mov eax, num2              ; get len of the second number
    call length                ; edi: ptr of num2
    mov edi, eax
    add edi, num2
    sub edi, 1

    mov ebx, operator
    cmp BYTE[ebx], 42
    jz .start_mul

.loopAdd:
    cmp esi, num1
    jl .rest_second_digits
    cmp edi, num2
    jl .rest_first_digits
    xor eax, eax
    add al, BYTE[esi]
    sub al, 48
    add al, BYTE[edi]           ; add by digits
    add al, cl                  ; add carry
    mov ecx, 0                  ; reset carry
    dec esi                     ; move ptr1
    dec edi                     ; move ptr2
    dec edx                     ; move result ptr
    cmp al, 57                  ; check if overflow occurs
    mov BYTE[edx], al
    jle .loopAdd                ; if not continue the loop
    call sub_1              ; if so, call sub digit
    mov BYTE[edx], al
    jmp .loopAdd

.rest_first_digits:
    cmp esi, num1
    jl .after_add
    xor eax, eax
    add al, BYTE[esi]           ; add by digits
    add al, cl                 ; add carry
    mov ecx, 0
    dec esi                     ; move ptr2
    dec edx                     ; move result ptr
    mov BYTE[edx], al
    cmp al, 57
    jle .rest_first_digits
    call sub_1
    mov BYTE[edx], al
    jmp .rest_first_digits

.rest_second_digits:
    cmp edi, num2
    jl .after_add
    xor eax, eax
    add al, BYTE[edi]           ; add by digits
    add al, cl                  ; add carry
    mov ecx, 0
    dec edi                     ; move ptr2
    dec edx                     ; move result ptr
    mov BYTE[edx], al
    cmp al, 57
    jle .rest_second_digits
    call sub_1
    mov BYTE[edx], al
    jmp .rest_second_digits

.add_carry:
    mov al, 49
    dec edx
    mov BYTE[edx], al
    jmp .print_sum

.after_add:
    cmp ecx, 1
    jz .add_carry
    jmp .print_sum

.print_sum:
    ;print str1
    mov eax, edx
    call printstr

    mov ebx, operator
    cmp BYTE[ebx], 43
    jz .loopstart

.start_mul:
    

    mov edx, product
    mov ecx, 0
    add edx, 255                ; edx: ptr of result
    xor eax, eax
    mov al, 10
    mov BYTE[edx], al
    dec edx

    mov eax, num1       ; get len of the first number
    call length                 ; esi: ptr of num1
    mov esi, eax
    add esi, num1
    sub esi, 1

    mov eax, num2      ; get len of the second number
    call length                 ; edi: ptr of num2
    mov edi, eax
    add edi, num2
    sub edi, 1                  ; from tail

.outter_loopMul:
.loop:
    cmp edi, num2       ; mul by digits
    jl .mul_output
    call inner_loopMul
    dec edi
    dec edx
    jmp .loop

.mul_output:
    mov edx, eax
    mov eax, str2
    ;print str2
    call format

.output_loop:
    cmp BYTE[edx], 48
    jnz .print_mul
    xor ecx, ecx
    add ecx, product
    add ecx, 254
    cmp edx, ecx
    je .print_mul
    add edx, 1
    jmp .output_loop

.print_mul:
    mov eax, edx
    call printstr

    jmp .loopstart
.endstart:
