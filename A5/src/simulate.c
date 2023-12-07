#include "./assembly.h"
#include "./simulate.h"
#include "./memory.h"
#include <stdint.h>

//DEFINING ENUM
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


//Returnerer antallet af instruktioner, som den har udført
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
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

    for (int i = start_addr; i <= start_addr + 0x10000; i+= 4) {
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
            /* code */
        } else if (opcode ^ LOAD_INST == 0x00) {
            /* code */
        } else if (opcode ^ STORE_INST == 0x00) {
            /* code */
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
