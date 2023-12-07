# GCC=gcc -g -Wall -Wextra -pedantic -std=gnu11 
GCC=gcc-13 -g -Wall -Wextra -pedantic -std=gnu11 -O
LD_FLAGS= -lpthread -lm

riscv:
	echo some
	
# % bruger som placeholder - dvs. en hvilken som helst fil med den endelse kan bruge dette target

%.riscv: %.c Makefile
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-gcc  -march=rv32im -mabi=ilp32 -fno-tree-loop-distribute-patterns -mno-relax -O1 $< -static -nostartfiles -nostdlib -o $@

#### DENNE KOMMANDO HERUNDER KAN BRUGES TIL AT LAVE EN ASSEMBLY FIL OM TIL EN .DIS FIL. DET KAN VI BRUGE NÅR VI SKAL TESTE VORES
#### INSTRUKTIONER SOM VI LAVER EN FOR EN:
#/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-as ass.s -o test.riscv

to_dis: *.s
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-as *.s -o $(basename $<).riscv

# to_dis: %.riscv: %.s
# 	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-as $< -o $@.riscv

%.dis: %.riscv Makefile
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-objdump -s -w $< > $@
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-objdump -S $< >> $@

all: sim

rebuild: clean all

# sim uses simulate
# sim: *.c 
# 	$(GCC) *.c -o sim 

sim: main.c simulate.c memory.c assembly.c read_exec.c
	$(GCC) $^ -o sim

zip: ../src.zip

../src.zip: clean
	cd .. && zip -r src.zip src/Makefile src/*.c src/*.h

clean:
	rm -rf *.o sim  vgcore*
