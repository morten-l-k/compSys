        .globl _start
_start:
        li r1, 7
        li r2, 2
        rem rd, r1, r2

        li a7, 93
        li a0, 0
        ecall