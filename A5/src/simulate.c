#include "./assembly.h"
#include "./simulate.h"
#include "./memory.h"
#include <stdint.h>

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
    SB = 0x00,
    SH = 0x01,
    SW = 0x02,
};


//Returnerer antallet af instruktioner, som den har udført
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    //TODO: IMPLEMENT PROGRAM COUNTER: program_counter = start_addr;
    int reg[REG_SIZE];
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
    
    int first_word = memory_rd_w(mem,start_addr);

    printf("first_word is: %d\n");
    printf("first_word opcode is: %d\n",first_word & 0x03);

    for (int i = start_addr; i <= start_addr + 0x10000; i += 4) {
        //FETCH INSTRUCTION (1/5)
        int inst_word = memory_rd_w(mem,i);

        //DECOMPOSE INSTRUCTION(2/5)
        uint32_t word = (uint32_t) inst_word;
        printf("Decomposed value is: %u\n",word);
        uint32_t opcode = (word << 25) >> 25;
        uint32_t func3 = (word << 17) >> 17;
        
        if (opcode ^ ECALL == 0x00) {
            /* code */
        } else if(opcode ^ LUI == 0x00) {
            //Implement code
        } else if (opcode ^ AUIPC == 0x00) {
            /* code */
        } else if (opcode ^ JAL == 0x00) {
            /* code */
        } else if (opcode ^ JALR == 0x00) {
            /* code */

        } else if (opcode ^ BRANCH_INST == 0x00) {
            if(func3 ^ BEQ == 0x00){
                //call BEQ
            } else if(func3 ^ BNE == 0x00) {
                //call BNE
            } else if(func3 ^ BLT == 0x00) {
                //call BLT
            } else if(func3 ^ BGE == 0x00) {
                //call BGE
            } else if(func3 ^ BLTU == 0x00) {
                //call BLTU
            } else if(func3 ^ BGEU == 0x00) {
                //call BGEU
            } else {
                printf("ERROR OCCURED - no such BRANCH instr. was found \n");
            }

        } else if (opcode ^ LOAD_INST == 0x00) {
            // TJEK OP PÅ BITSHIFTENE HER OG HVOVIDT DE FORSKELLIGE DELE ER SIGN EXTENDED ELLER EJ
            uint32_t base = (word << 12) >> 27;
            uint32_t rd = (word << 20) >> 27;
            int32_t offset = word >> 20;

            if(func3 ^ LB == 0x00){
                // sign extend the 8 bits to 32 bits
                int32_t value = memory_rd_b(mem,reg[base] + offset);
                reg[rd] = value;  
            } else if(func3 ^ LH == 0x00) {
                // sign extend the 16 bits to 32 bits 
                int32_t value = memory_rd_h(mem,reg[base] + offset); 
                reg[rd] = value;
            } else if(func3 ^ LW == 0x00) {
                reg[rd] = memory_rd_w(mem,reg[base] + offset); 
            } else if(func3 ^ LBU == 0x00) {
                // zero extend the 8 bits to 32 bits
                uint32_t value = memory_rd_b(mem,reg[base] + offset);
                reg[rd] = value;
            } else if(func3 ^ LHU == 0x00) {
                // zero extend the 16 bits to 32 bits
                uint32_t value = memory_rd_h(mem, reg[base] + offset);
                reg[rd] = value;
            } else {
                printf("ERROR OCCURED - no such LOAD instr. was found \n");
            }

        } else if (opcode ^ STORE_INST == 0x00) {
            uint32_t base = (word << 12) >> 27;
            uint32_t src = (word << 7) >> 27;
            uint32_t offset_0_4 = (word << 20) >> 27;
            uint32_t offset_5_11 = (word >> 25) << 5;
            uint32_t total_offset = offset_0_4 + offset_5_11;

            if (func3 ^ SB) {
                //UKLART OM MEMORY_WR_H SELV HÅNDTERER KUN AT GEMME EN BYTE
                memory_wr_b(mem,reg[base] + total_offset, reg[src]);
            } else if (func3 ^ SH) {
                //UKLART OM MEMORY_WR_H SELV HÅNDTERER KUN AT GEMME ET HALF-WORD
                memory_wr_h(mem,reg[base] + total_offset,reg[src]);
            } else if (func3 ^ SW) {
                memory_wr_w(mem,reg[base] + total_offset,reg[src]);
            }
            
        } else if (opcode ^ IMMEDIATE_INST == 0x00) {
            /* code */
        } else if (opcode ^ RTYPE_INST == 0x00) {
            /* code */
        } else {
            printf("ERROR OCCURED\n");
        }

        //EXECUTE (3/5)


        //MEMORY (4/5) 


        //WRITEBACK (5/5)
    }
}
