#include "./assembly.h"
#include "./simulate.h"
#include "./memory.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

//TODO: IMPLEMENT PROGRAM COUNTER: program_counter = 0;

//DEFINING ENUM OPCODE
enum Opcode {
    ECALL = 0x73,
    LUI = 0x37,
    AUIPC = 0x17,
    JAL = 0x6f,
    JALR = 0x67,
    BRANCH_INST = 0x63, //Includes instructions: BEQ, BNE, BLT, BGE, BLTU, BGEU
    LOAD_INST = 0x03, //Includes instructions: LB, LH, LW, LBU, LHU
    STORE_INST = 0x23, //Includes instructions: SB, SH, SW
    IMMEDIATE_INST = 0x13, //Includes instructions: ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI...
    RTYPE_INST = 0x33, //Includes instructions: ADD, SUB, SLL, SLT and also MUL instructions...
};

enum BranchInstructions {
    BEQ = 0x00,
    BNE = 0x01,
    BLT = 0x04,
    BGE = 0x05,
    BLTU = 0x06,
    BGEU = 0x07,
};

enum LoadInstructions {
    LB = 0x00,
    LH = 0x01,
    LW = 0x02,
    LBU = 0x04,
    LHU = 0x05,
};

enum StoreInstructions {
    //Func3 bit seqeunces
    SB = 0x00,
    SH = 0x01,
    SW = 0x02,
};

enum ImmediateInstructions {
    ADDI = 0x00,
    SLTI = 0x02,
    SLTIU = 0x03,
    XORI = 0x04,
    ORI = 0x06,
    ANDI = 0x07,
    SLLI = 0x01,
};

enum RtypeInstructions {
    //Func7 bit sequences
    RTYPE_0 = 0x00,
    RTYPE_1 = 0x20, //Includes SUB, SRA. Thus RTYPE_1 has value 0x02 in func7
    MUL_TYPE = 0x01,
    //Func3 bit sequences of RTYPE_0 and RTYPE_1
    ADD = 0x00, 
    SUB = 0x00,//Note: SUB is RTYPE_1
    SLL = 0x01,
    SLT = 0x02,
    SLTU = 0x03,
    XOR = 0x04,
    SRL = 0x05,
    SRA = 0x05, //Note: SRA is RTYPE_1
    OR = 0x06,
    AND = 0x07,
    //Func3 bit sequences of mul
    MUL = 0x00,
    MULH = 0x01,
    MULHSU = 0x02,
    MULHU = 0x03,
    DIV = 0x04,
    DIVU = 0x05,
    REM = 0x06,
    REMU = 0x07,
};

enum ImmediateInstructions {
    //Func3
    ADDI = 0x00,
    SLTI = 0x02,
    SLTIU = 0x03,
    XORI = 0x04,
    ORI = 0x06,
    ANDI = 0x07,
    SLLI = 0x01,
    SRLI_SRAI = 0x05,
    //Func7
    SRLI = 0x00,
    SRAI = 0x20,
};



//Returnerer antallet af instruktioner, som den har udført
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    //ASSEMBLY-FILEN KAN BARE BRUGES TIL AT FÅ EN GIVEN ASSEMBLER-KODE UD FRA EN GIVEN ADDRESSE.
    //SÅ DVS. DET ER BARE EN NEMMERE MÅDE AT FÅ ASSEMBLE PÅ
    //TODO: IMPLEMENT PROGRAM COUNTER: program_counter = start_addr;
    int reg[REG_SIZE]; //TODO: Beslut om register skal være af uint32_t typer i stedet
    const int hard_wired_zero = 0;
    reg[zero] = hard_wired_zero;

    //Vi skal ikke ændre i memory.c, read_exec.c eller anden. Kun her i simulate!

    //Så dvs. vores C-simulator skal altså kunne håndtere de instruktioner, som vi får givet, 
    //således at den korrekte brug af read/write til fx memory håndteres korrekt. 

    //C programmet skal kunne læse en række værdier fra lageret - vi får et tal, og så skal vi 
    //fortolke det tal til en given instruktion, som så (muligvis) vil tage noget data fra en given
    //addresse i memory.

    //read_exec leder efter _GLOBAL_START for at finde hvor den skal begynde at læse fra. Men den læser
    //ikke assembly kommandoer, men loader bare værdierne fra en .dis fil ind i memory. Det er kun det, 
    //den gør.

    //Ved test af vores egen kode, så lav simple assebmly programmer på få linjer til at teste en eller få instruktioner
    
    for (int i = start_addr; i <= start_addr + 0x10; i += 4) {
        //FETCH INSTRUCTION (1/5)
        int inst_word = memory_rd_w(mem,i);

        //DECOMPOSE INSTRUCTION(2/5)
        uint32_t word = (uint32_t) inst_word;
        printf("Decomposed value is: 0x%08x\n",word);
        uint32_t opcode = (word << 25) >> 25;
        uint32_t func3 = (word << 17) >> 17;
        printf("Opcode is: 0x%08x\n",opcode);
        // obs 
        if ((opcode ^ ECALL) == 0x00) {
            if (reg[a7] == 1) {
                reg[a0] = getchar();
            } else if (reg[a7] == 2) {
                putchar(reg[a0]);
            } else {
                printf("About to exit_Failure\n");
                return EXIT_FAILURE;
            }
            /* code */
        } else if((opcode ^ LUI) == 0x00) {
            //Implement code
        } else if ((opcode ^ AUIPC) == 0x00) {
            /* code */
        } else if ((opcode ^ JAL) == 0x00) {
            /* code */
        } else if ((opcode ^ JALR) == 0x00) {
            /* code */
        } else if ((opcode ^ BRANCH_INST) == 0x00) {
            uint32_t imm_1_4 = (((word << 20) >> 28) << 1);
            uint32_t imm_5_10 = (((word << 1) >> 26) << 5);
            uint32_t imm_11 = (((word << 24) >> 31) << 10);
            uint32_t tmp_offset = imm_1_4 + imm_5_10 + imm_11;
            uint32_t signbit = word >> 31;
            uint32_t total_offset = 0x00000000;
            //Padding leftmost bits if signbit is negative
            if (signbit == 0x01) {
                total_offset = 0xfffff000 + tmp_offset; 
            } else {
                total_offset = tmp_offset;
            }
            
            uint32_t rs1 = (word << 12) >> 27;
            uint32_t rs2 = (word << 7) >> 27;

            if((func3 ^ BEQ) == 0x00){
                if (reg[rs1] == reg[rs2]) {
                    //PC + total_offset
                }
            } else if((func3 ^ BNE) == 0x00) {
                //call BNE
            } else if((func3 ^ BLT) == 0x00) {
                //call BLT
            } else if((func3 ^ BGE) == 0x00) {
                //call BGE
            } else if((func3 ^ BLTU) == 0x00) {
                //call BLTU
            } else if((func3 ^ BGEU) == 0x00) {
                //call BGEU
            } else {
                printf("ERROR OCCURED - no such BRANCH instr. was found \n");
            }

        } else if ((opcode ^ LOAD_INST) == 0x00) {
            // TJEK OP PÅ BITSHIFTENE HER OG HVORVIDT DE FORSKELLIGE DELE ER SIGN EXTENDED ELLER EJ
            uint32_t base = (word << 12) >> 27;
            uint32_t rd = (word << 20) >> 27;
            int32_t offset = word >> 20;

            if((func3 ^ LB) == 0x00){
                // sign extend the 8 bits to 32 bits
                int32_t value = memory_rd_b(mem,reg[base] + offset);
                reg[rd] = value;  
            } else if((func3 ^ LH) == 0x00) {
                // sign extend the 16 bits to 32 bits 
                int32_t value = memory_rd_h(mem,reg[base] + offset); 
                reg[rd] = value;
            } else if((func3 ^ LW) == 0x00) {
                reg[rd] = memory_rd_w(mem,reg[base] + offset); 
            } else if((func3 ^ LBU) == 0x00) {
                // zero extend the 8 bits to 32 bits
                uint32_t value = memory_rd_b(mem,reg[base] + offset);
                reg[rd] = value;
            } else if((func3 ^ LHU) == 0x00) {
                // zero extend the 16 bits to 32 bits
                uint32_t value = memory_rd_h(mem, reg[base] + offset);
                reg[rd] = value;
            } else {
                printf("ERROR OCCURED - no such LOAD instr. was found \n");
            }

        } else if ((opcode ^ STORE_INST) == 0x00) {
            uint32_t base = (word << 12) >> 27;
            uint32_t src = (word << 7) >> 27;
            uint32_t offset_0_4 = (word << 20) >> 27;
            uint32_t offset_5_11 = (word >> 25) << 5;
            uint32_t total_offset = offset_0_4 + offset_5_11;

            if ((func3 ^ SB) == 0x00) {
                //UKLART OM MEMORY_WR_H SELV HÅNDTERER KUN AT GEMME EN BYTE
                memory_wr_b(mem,reg[base] + total_offset, reg[src]);
            } else if ((func3 ^ SH) == 0x00) {
                //UKLART OM MEMORY_WR_H SELV HÅNDTERER KUN AT GEMME ET HALF-WORD
                memory_wr_h(mem,reg[base] + total_offset,reg[src]);
            } else if ((func3 ^ SW) == 0x00) {
                memory_wr_w(mem,reg[base] + total_offset,reg[src]);
            } else {
                printf("Error occured in STORE_INST\n");
            }
            
        } else if ((opcode ^ IMMEDIATE_INST) == 0x00) {
            uint32_t func7 = word >> 25;
            uint32_t rs1 = (word << 12) >> 27;
            uint32_t shamt = (word << 7) >> 27;
            uint32_t rd = (word << 20) >> 27;
            uint32_t imm_11_0 = word >> 20;
            int signed_imm_11_0 = ((int)word) >> 20;
            uint32_t signbit = word >> 31;
          
           uint32_t base = (word >> 7) & 0x1F;
            uint32_t src = (word >> 15) & 0x1F;
            int32_t immVal = (word >> 20) & 0xFFF;

             if ((func3 ^ ADDI) == 0x00) {
                reg[base] = reg[src] + immVal;
                }
            // SLTI = set less than immediate
            else if ((func3 ^ SLTI) == 0x00) {
                reg[base] = (reg[src] < immVal) ? 1:0;
            }
            //SLTIU = Set Less Than Immediate Unsigned
            else if ((func3 ^ SLTIU )  == 0x00){
                reg[base] = (reg[src] < (uint32_t)immVal) ? 1:0;
            } 
            else if ((func3 ^ XORI) == 0x00) {
                if (signbit == 0x00) {
                    reg[rd] = ((uint32_t)reg[rs1]) ^ imm_11_0;
                } else {
                    reg[rd] = ((int)reg[rs1]) ^ signed_imm_11_0;
                }
            } else if ((func3 ^ ORI) == 0x00) {
                if (signbit == 0x00) {
                    reg[rd] = ((uint32_t)reg[rs1]) | imm_11_0;
                } else {
                    reg[rd] = ((int)reg[rs1]) | signed_imm_11_0;
                }
            } else if ((func3 ^ ANDI) == 0x00) {
                if(signbit == 0x00) {
                    reg[rd] = ((uint32_t)reg[rs1]) & imm_11_0;
                } else {
                    reg[rd] = ((int)reg[rs1]) & signed_imm_11_0;
                }
            } else if ((func3 ^ SLLI) == 0x00) {
                reg[rd] = ((int)reg[rs1]) << shamt;
            } else if ((func3 ^ SRLI_SRAI) == 0x00) {
                if ((func7 ^ SRLI) == 0x00) {
                    reg[rd] = ((uint32_t)reg[rs1]) >> shamt;
                } else if ((func7 ^ SRAI) == 0x00) {
                    if (signbit == 0x00) {
                        reg[rd] = ((uint32_t)reg[rs1]) >> shamt;
                    } else {
                        reg[rd] = ((int)reg[rs1]) >> shamt;
                    }
                }
            } else {
                printf("Error occured in IMMEDIATE_INST\n");
            }
            
        } else if ((opcode ^ RTYPE_INST) == 0x00) {
            uint32_t func7 = word >> 25;
            uint32_t rs1 = (word << 12) >> 27;
            uint32_t rs2 = (word << 7) >> 27;
            uint32_t rd = (word << 20) >> 27;

            //RTYPE_0 (dvs. instruktioner hvor func7 = 0x00)
            if ((func7 ^ RTYPE_0) == 0x00) {
                if (func3 ^ ADD) {
                    reg[rd] = reg[rs1] + reg[rs2];
                } else if ((func3 ^ SLL) == 0x00) {
                    reg[rd] = reg[rs1] << reg[rs2];
                } else if ((func3 ^ SLT) == 0x00) {
                    if (reg[rs1] < reg[rs2]) {
                        reg[rd] = 0x00000001;
                    } else {
                        reg[rd] = 0x00;
                    }
                } else if ((func3 ^ SLTU) == 0x00) {
                    if (((uint32_t)reg[rs1]) < ((uint32_t)reg[rs2])) {
                        reg[rd] = 0x00000001;
                    } else {
                        reg[rd] = 0x00;
                    }
                } else if ((func3 ^ XOR) == 0x00) {
                    reg[rd] = reg[rs1] ^ reg[rs2];
                } else if ((func3 ^ SRL) == 0x00) {
                    reg[rd] = reg[rs1] >> reg[rs2];
                } else if ((func3 ^ OR) == 0x00) {
                    reg[rd] = reg[rs1] | reg[rs2];
                } else if ((func3 ^ AND) == 0x00) {
                    reg[rd] = reg[rs1] & reg[rs2];
                } else {
                    printf("Error in IMMEDIATE_RTYP0 instructions\n");
                }
                
            }
            //RTYPE_1 (dvs. instruktioner hvor func7 = 0x20) 
            else if((func7 ^ RTYPE_1) == 0x00) {
                if ((func3 ^ SUB) == 0x00) {
                    reg[rd] = reg[rs1] - reg[rs2];
                } else if ((func3 ^ SRA) == 0x00) {
                    uint32_t signbit = ((uint32_t)reg[rs1]) >> 31;
                    if (signbit == 0x00) {
                        reg[rd] = ((uint32_t)reg[rs1]) >> reg[rs2];
                    } else if (signbit == 0x01) {
                        reg[rd] = ((int)reg[rs1]) >> reg[rs2];
                    } else {
                        printf("Error occured in RTYPE_1_INSTR\n");
                    }
                } else {
                    printf("Error occured in RTYPE_1_INST\n");
                }
                
                

            } else if((func7 ^ MUL_TYPE) == 0x00) {
                if ((func3 ^ MUL) == 0x00) {
                    uint64_t res = (uint64_t)reg[rs2] * (uint64_t)reg[rs1];
                    reg[rd] = res;
                }
                

            } else {
                printf("Error occured in RTYPE_INST\n");
            }
            


            /* code */
        } else {
            printf("ERROR OCCURED in RTYPE\n");
        }

        //EXECUTE (3/5)


        //MEMORY (4/5) 


        //WRITEBACK (5/5)
    }
}
