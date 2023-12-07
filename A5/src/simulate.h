#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include <stdio.h>

#define REG_SIZE    32
#define zero        0 //Hard-wired zero
#define ra          1 //Return address
#define sp          2 //stack pointer
#define gp          3 //global pointer
#define tp          4 //Thread pointer
//Temporaries
#define t0          5
#define t1          6
#define t2          7
//Frame pointer
#define fp          8
//Saved register
#define sr          9
//Return values/Function arguments
#define a0          10
#define a1          11
//Function arguments
#define a2          12
#define a3          13
#define a4          14
#define a5          15
#define a6          16
#define a7          17
//Saved registers
#define s2          18
#define s3          19
#define s4          20
#define s5          21
#define s6          22
#define s7          23
#define s8          24
#define s9          25
#define s10         26
#define s11         27
//Temporaries
#define t3          28
#define t4          29
#define t5          30
#define t6          31

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
