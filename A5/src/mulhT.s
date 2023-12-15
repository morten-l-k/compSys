        .globl _start
_start:
    li r1, 5
    li r2, 10
    mulh rd, r1, r2

    li a7, 93
    li a0, 0
    ecall


