.syntax unified

.global asm_sub
.global asm_led
.global add_overflow
.global add_6
.global asm_error


asm_sub:
    sub r0, r0, r1 //mov r0,2 mov r1,1 by default
    bx lr    //PC = register lr  return to the beginning address stored in register lr and return r0


asm_led:
    cmp r0, #0
    beq isZero
    sub r0,r0,#1
    bx lr
    isZero:
    add r0,r0,#1
    bx lr

asm_error:
    adds r0, r0, r1
    bvs vFlagOn   //vs: overflow set  vc: overflow clear
    mov r0, #0
    bx lr
    vFlagOn:
    mov r0,#1
    bx lr

add_6:
    push {r11}  //equals to str r11, [sp,#-4]
    add r11, sp,#0
    add r0, r0, r1 
    add r0, r0, r2
    add r0, r0, r3  // only 4 parameters can be store in r0 to r3, other parameters are stored in stack, e stored in sp+4, f stored in sp+8 by default
    ldr r1, [r11, #4] //load the value stored in sp+4 to r1. sp is a register stores the address of where stack ends. 
    add r0, r0, r1 
    ldr r1, [r11, #8]
    add r0, r0, r1 
    pop {r11}  //equals to ldr r11, [sp,#-4]
    bx lr




    


