#include "spimcore.h"
#define MEMSIZE 65536 // Define MEMSIZE as 64KB
/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {
    switch (ALUControl) {
        case 0: *ALUresult = A + B; break;
        case 1: *ALUresult = A - B; break;
        case 2: *ALUresult = ((int)A < (int)B) ? 1 : 0; break;
        case 3: *ALUresult = (A < B) ? 1 : 0; break;
        case 4: *ALUresult = A & B; break;
        case 5: *ALUresult = A | B; break;
        case 6: *ALUresult = B << 16; break;
        case 7: *ALUresult = ~A; break;
    }
    *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // Checking for word alignment, if not a multie of 4 it means it is not word aligned
    if (PC % 4 != 0)
    {
        return -1;
    }

    // Fetching the instruction from memory
    *instruction = Mem[PC >> 2];

    return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec) {
    *op = (instruction >> 26) & 0x3F;
    *r1 = (instruction >> 21) & 0x1F;
    *r2 = (instruction >> 16) & 0x1F;
    *r3 = (instruction >> 11) & 0x1F;
    *funct = instruction & 0x3F;
    *offset = instruction & 0xFFFF;
    *jsec = instruction & 0x3FFFFFF;
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
    switch(op) {
        case 0x00: // R-type
            controls->RegDst = 1;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 7;
            controls->MemWrite = 0;
            controls->ALUSrc = 0;
            controls->RegWrite = 1;
            return 0;

        case 0x23: // lw
            controls->RegDst = 0;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 1;
            controls->MemtoReg = 1;
            controls->ALUOp = 0;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            return 0;

        case 0x2B: // sw
            controls->RegDst = 2;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 2;
            controls->ALUOp = 0;
            controls->MemWrite = 1;
            controls->ALUSrc = 1;
            controls->RegWrite = 0;
            return 0;

        case 0x04: // beq
            controls->RegDst = 2;
            controls->Jump = 0;
            controls->Branch = 1;
            controls->MemRead = 0;
            controls->MemtoReg = 2;
            controls->ALUOp = 1;
            controls->MemWrite = 0;
            controls->ALUSrc = 0;
            controls->RegWrite = 0;
            return 0;

        case 0x02: // jump
            controls->RegDst = 2;
            controls->Jump = 1;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 2;
            controls->ALUOp = 0;
            controls->MemWrite = 0;
            controls->ALUSrc = 0;
            controls->RegWrite = 0;
            return 0;

        default:
            return 1;
    }
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}



/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value) {
    *extended_value = (offset & 0x8000) ? (offset | 0xFFFF0000) : (offset & 0x0000FFFF);
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero) {
    unsigned operandB = ALUSrc ? extended_value : data2;

    switch(ALUOp) {
        case 0: ALU(data1, operandB, 0, ALUresult, Zero); break;
        case 1: ALU(data1, operandB, 1, ALUresult, Zero); break;
        case 2: ALU(data1, operandB, 2, ALUresult, Zero); break;
        case 3: ALU(data1, operandB, 3, ALUresult, Zero); break;
        case 4: ALU(data1, operandB, 4, ALUresult, Zero); break;
        case 5: ALU(data1, operandB, 5, ALUresult, Zero); break;
        case 6: ALU(data1, operandB, 6, ALUresult, Zero); break;
        case 7:
            switch(funct) {
                case 32: ALU(data1, data2, 0, ALUresult, Zero); break; // add
                case 34: ALU(data1, data2, 1, ALUresult, Zero); break; // sub
                case 36: ALU(data1, data2, 4, ALUresult, Zero); break; // and
                case 37: ALU(data1, data2, 5, ALUresult, Zero); break; // or
                case 42: ALU(data1, data2, 2, ALUresult, Zero); break; // slt
                default: return 1;
            }
            break;
        default: return 1;
    }
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

    if(MemWrite == 1)
    {
        // Check word alignment
        if(ALUresult % 4 != 0)
        {
            return -1;
        }

        // write into memory
        Mem[ALUresult >> 2] = data2;
    }


    if (MemRead == 1)
    {
        // Check word alignment
        if(ALUresult % 4 != 0)
        {
            return -1;
        }

        // read from memory
        *memdata = Mem[ALUresult >> 2];
    }

    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

    if(RegWrite == 1)
    {
        if(MemtoReg == 1)
        {
            unsigned data;

            // If MemtoReg is 1 && RegWrite is 1 then data is coming from memory (memdata)
            if(MemtoReg == 1)
            {
                data = memdata;
            }
            else
            {
                // If MemtoReg is 0 && RegWrite is 1 then data is coming from ALU_result (ALUresult)
                data = ALUresult;
            }

            

            // Determine the register destination
            
            unsigned index;
            if(RegDst == 1)
            {
                index = r3;
            }
            else
            {
                index = r2;
            }

            Reg[index] = data;
        }
    }

}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
    if (Jump) {
        *PC = ((*PC & 0xF0000000) | (jsec << 2));
    } else if (Branch && Zero) {
        *PC += (extended_value << 2);
    } else {
        *PC += 4;
    }
}

