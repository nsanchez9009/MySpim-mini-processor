//Nicolas Sanchez
//CDA 3103
//Project

#include "spimcore.h"

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch(ALUControl)
    {
        //A + B.
        case 0x0:
            *ALUresult = A + B;
            break;

        //A - B.
        case 0x1:
            *ALUresult = A - B;
            break;

        //If A < B, set Z to 1.
        //If not set it to 0.
        //Cast them to int because this instruction is not unsigned.
        case 0x2:
            if ((int)A < (int)B)
            {
                *ALUresult = 1;
            }
            else
            {
                *ALUresult = 0;
            }
            break;

        //If A < B, set Z to 1.
        //If not set it to 0.
        //No need to cast here.
        case 0x3:
            if (A < B)
            {
                *ALUresult = 1;
            }
            else
            {
                *ALUresult = 0;
            }

            break;

        //Both A and B.
        case 0x4:
            *ALUresult = A & B;
            break;

        //A or B.
        case 0x5:
            *ALUresult = A | B;
            break;

        //Shift B to the left 16 bits.
        case 0x6:
            *ALUresult = B << 16;
            break;

        //Not A.
        case 0x7:
            *ALUresult = ~A;
            break;
    }

    //CHecking for zeros.
    //If it equals 0, set it to 1. If not, set it to 0.
    if (*ALUresult == 0)
    {
        *Zero = 1;
    }
    else
    {
        *Zero = 0;
    }
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    //Halt condition.
    if(PC % 4 != 0)
    {
        return 1;
    }

    *instruction = Mem[PC >> 2];

    return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    //[31-26]
    *op = (instruction & 0xfc000000) >> 26;

    //[25-21]
    *r1 = (instruction & 0x03e00000) >> 21;

    //[20-16]
    *r2 = (instruction & 0x001f0000) >> 16;

    //[15-11]
    *r3 = (instruction & 0x0000f800) >> 11;

    //[5-0]
    *funct = instruction & 0x0000003f;

    //[15-0]
    *offset = instruction & 0x0000ffff;

    //[25-0]
    *jsec = instruction & 0x03ffffff;
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    //Set all controls to 0.
    //This is done in order to shorten and reduce the size of this switch statement. Reducing repeated code.
    controls->RegDst = 0;
	controls->Jump = 0;
	controls->Branch = 0;
	controls->MemRead = 0;
	controls->MemtoReg = 0;
	controls->ALUOp = 0;
	controls->MemWrite = 0;
	controls->ALUSrc = 0;
	controls->RegWrite = 0;

    switch(op)
    {
        //R-type.
        case 0x0:
            controls->RegDst = 1;
            controls->ALUOp = 7;
            controls->RegWrite = 1;

            break;

        //Addi.
        case 0x8:
            controls->RegWrite = 1;
            controls->ALUSrc = 1;

            break;

        //Lw.
        case 0x23:
            controls->RegWrite = 1;
            controls->MemRead = 1;
            controls->MemtoReg = 1;
            controls->ALUSrc = 1;
            break;

        //Sw.
        case 0x2b:
            controls->MemWrite = 1;
            controls->RegDst = 2;
            controls->MemtoReg = 2;
            controls->ALUSrc = 1;

            break;

        //Lui.
        case 0xf:
            controls->RegWrite = 1;
            controls->ALUOp = 6;
            controls->ALUSrc = 1;

            break;

        //Beq.
        case 0x4:
            controls->Branch = 1;
            controls->RegDst = 2;
            controls->MemtoReg = 2;
            controls->ALUSrc = 1;
            controls->ALUOp = 1;

            break;

        //Slti.
        case 0xa:
            controls->ALUOp = 2;
            controls->RegWrite = 1;
            controls->ALUSrc = 1;

            break;

        //Sltiu.
        case 0xb:
            controls->ALUOp = 3;
            controls->RegWrite = 1;
            controls->ALUSrc = 1;

            break;

        //Jump.
        case 0x2:
            controls->Jump = 1;
            controls->RegDst = 2;
            controls->Branch = 2;
            controls->MemtoReg = 2;
            controls->ALUSrc = 2;
            controls->ALUOp = 2;

            break;

        //If none of these cases are met, halt.
        default:
            return 1;
    }

    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    //Index to r1 and r2 within reg and save the values to data1 and 2.
    *data1 = Reg[r1];
	*data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    //If offset is negative.
    if((offset >> 15) == 1)
    {
        //fill with 1's.
        *extended_value = offset | 0xffff0000;
    }

    //Else (offset is positive).
    else
    {
        //Fill with 0's.
        *extended_value = offset & 0x0000ffff;
    }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    //If ALUOp does not equal 7, halt.
    if(ALUOp < 0 || ALUOp > 7)
    {
        return 1;
    }

    else if (ALUSrc == 1)
    {
        data2 = extended_value;
    }
    
    //ALUOp 7 is an r type instruction.
    else if(ALUOp == 7)
    {
        switch(funct)
        {
            //Add.
            case 0x20:
                ALUOp = 0;
                break;
                
            //Subtract.
            case 0x22:
                ALUOp = 1;
                break;

            //Set less than.
            case 0x2a:
                ALUOp = 2;
                break;

            //Set less than unsigned.
            case 0x2b:
                ALUOp = 3;
                break;

            //And.
            case 0x24:
                ALUOp = 4;
                break;

            //Or.
            case 0x25:
                ALUOp = 5;
                break;

            //Halt.
            default:
                return 1;
        }
        ALU(data1, data2, ALUOp, ALUresult, Zero);
    }

    //Non funct.
    else
    {
        ALU(data1, data2, ALUOp, ALUresult, Zero);
    }

    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    //If writing to memory.
    if(MemWrite == 1)
    {
        if(ALUresult % 4 == 0)
        {
            Mem[ALUresult >> 2] = data2;
        }

        //Halt.
        else
        {
            return 1;
        }
    }

    //If reading from memory.
    else if(MemRead == 1)
    {
        if(ALUresult % 4 == 0)
        {
            *memdata = Mem[ALUresult >> 2];
        }

        //Halt.
        else
        {
            return 1;
        }
    }

    return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    //If writing.
    if(RegWrite == 1)
    {
        //Memory to register.
        if (MemtoReg == 1)
        {
            Reg[r2] = memdata;
        }

        else if (MemtoReg == 0)
        {
            //R type.
            if(RegDst == 1)
            {
               Reg[r3] = ALUresult; 
            }

            //If not, I type.
            else
            {
                Reg[r2] = ALUresult;
            }
        }
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC += 4;

    //If branch and zero.
    if(Branch == 1 && Zero == 1)
    {
        *PC += extended_value << 2;
    }

    //If Jump.
    if(Jump == 1)
    {
        *PC = (*PC & 0xf000000) | (jsec << 2);
    }
}   

