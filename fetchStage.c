#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "printInternalReg.h"

#define ERROR_RETURN -1
#define SUCCESS 0

int numConsecutiveHalt = 0;

unsigned long long computeResult(char* buf, int counter){
    unsigned long long byte0 = (buf[counter+2] & 0xff);
    unsigned long long byte1 = (buf[counter+3] & 0xff);
    unsigned long long byte2 = (buf[counter+4] & 0xff);
    unsigned long long byte3 = (buf[counter+5] & 0xff);
    unsigned long long byte4 = (buf[counter+6] & 0xff);
    unsigned long long byte5 = (buf[counter+7] & 0xff);
    unsigned long long byte6 = (buf[counter+8] & 0xff);
    unsigned long long byte7 = (buf[counter+9] & 0xff);
    byte1 = byte1 << 8;
    byte2 = byte2 << 16;
    byte3 = byte3 << 24;
    byte4 = byte4 << 32;
    byte5 = byte5 << 40;
    byte6 = byte6 << 48;
    byte7 = byte7 << 56;
    
    unsigned long long result = byte0 + byte1 + byte2 + byte3 + byte4 + byte5 + byte6 + byte7;
    
    return result;
}

int main(int argc, char **argv){


    int machineCodeFD = -1;       // File descriptor of file with object code
    uint64_t PC = 0;              // The program PC
    struct fetchRegisters fReg;

    int buffer_Size = 65536;
    int size;
    char src_buff[buffer_Size];

    // Verify that the command line has an appropriate number
    // of arguments

    if (argc < 2 || argc > 3) {
        printf("Usage: %s InputFilename [startingOffset]\n", argv[0]);
        return ERROR_RETURN;
    }

    // First argument is the file to open, attempt to open it
    // for reading and verify that the open did occur.
    machineCodeFD = open(argv[1], O_RDONLY);

    if (machineCodeFD < 0){
        printf("Failed to open: %s\n", argv[1]);
        return ERROR_RETURN;
    }

    // If there is a 2nd argument present, it is an offset so
    // convert it to a value. This offset is the initial value the
    // program PC is to have. The program will seek to that location
    // in the object file and begin fetching instructions from there.
    if (3 == argc) {
        // See man page for strtol() as to why
        // we check for errors by examining errno
        errno = 0;
        PC = strtol(argv[2], NULL, 0);
        if (errno != 0) {
            perror("Invalid offset on command line");
            return ERROR_RETURN;
        }
    }

    printf("Opened %s, starting offset 0x%016llX\n", argv[1], PC);

    // Start adding your code here and comment out the line the #define

    // http://stackoverflow.com/questions/18985488/printing-from
    //  -a-buffer-using-the-read2-function-in-c
    ssize_t real_read_len = read(machineCodeFD,src_buff,sizeof(src_buff));;
    char* ptr = src_buff;
    if(real_read_len > 0){
        int counter;
        unsigned long long result;
        while(PC <= real_read_len){
            int icode = (ptr[PC] >> 4) & 0x0F;
            if(icode == 0){
                numConsecutiveHalt++;
            }
            else{
                numConsecutiveHalt = 0;
            }

            switch(icode){
                case 0:
                counter = PC;
                if(real_read_len - counter >= 1){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 0;
                    fReg.valCValid = 0;
                    fReg.valP = PC + 1;
                    fReg.instr = "halt";
                    if(numConsecutiveHalt <= 2){
                        printRegS(&fReg);
                    }
                    PC += 1;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 1, real_read_len - counter);
                    exit(0);
                }
                break;

                case 1:
                counter = PC;
                if(real_read_len - counter >= 1){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 0;
                    fReg.rA = 0xF;
                    fReg.rB = 0xF;
                    fReg.valCValid = 0;
                    fReg.valP = PC + 1;
                    fReg.instr = "nop";
                    printRegS(&fReg);
                    PC += 1;
                }
                else{
                    // printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 1, real_read_len - counter);
                    exit(0);
                }
                break;

                case 2:
                counter = PC;
                if(real_read_len - counter >= 2){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.ifun = ptr[PC] & 0x0F;
                    fReg.regsValid = 1;
                    fReg.rA = (ptr[PC+1] >> 4) & 0x0F;
                    fReg.rB = ptr[PC+1] & 0x0F;
                    if(fReg.rA == 0xf || fReg.rB == 0xf){
                        printf("Invalid register");
                        exit(0);
                    }
                    fReg.valCValid = 0;
                    fReg.valP = PC + 2;
                    switch(fReg.ifun){
                        case 0:
                        fReg.instr = "rrmovq";
                        printRegS(&fReg);
                        break;
                        case 1:
                        fReg.instr = "cmovle";
                        printRegS(&fReg);
                        break;
                        case 2:
                        fReg.instr = "cmovl";
                        printRegS(&fReg);
                        break;
                        case 3:
                        fReg.instr = "cmove";
                        printRegS(&fReg);
                        break;
                        case 4:
                        fReg.instr = "cmovge";
                        printRegS(&fReg);
                        break;
                        case 5:
                        fReg.instr = "cmovge";
                        printRegS(&fReg);
                        break;
                        case 6:
                        fReg.instr = "cmovg";
                        printRegS(&fReg);
                        break;
                        default:
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    PC += 2;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 2, real_read_len - counter);
                    exit(0);
                }
                break;

                case 3:
                counter = PC;
                if(real_read_len - counter >= 10){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.ifun = ptr[PC] & 0x0f;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 1;
                    fReg.rA = (ptr[PC+1] >> 4) & 0x0F;
                    if(fReg.rA != 0x0f){
                        printf("Invalid register");
                        exit(0);
                    }
                    fReg.rB = ptr[PC+1] & 0x0F;
                    fReg.valCValid = 1;

                    result = computeResult(ptr, counter);
                    fReg.valC = result;

                    fReg.byte0 = ptr[PC+2];
                    fReg.byte1 = ptr[PC+3];
                    fReg.byte2 = ptr[PC+4];
                    fReg.byte3 = ptr[PC+5];
                    fReg.byte4 = ptr[PC+6];
                    fReg.byte5 = ptr[PC+7];
                    fReg.byte6 = ptr[PC+8];
                    fReg.byte7 = ptr[PC+9];
                    fReg.valP = PC + 10;
                    fReg.instr = "irmovq";
                    printRegS(&fReg);
                    PC += 10;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required %d bytes, read %ld bytes.", PC, 10, real_read_len - counter);
                    exit(0);
                }
                break;

                case 4:
                counter = PC;
                if(real_read_len - counter >= 10){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 1;
                    fReg.rA = (ptr[PC+1] >> 4) & 0x0F;
                    fReg.rB = ptr[PC+1] & 0x0F;
                    if(fReg.rA == 0xf || fReg.rB == 0xf){
                        printf("Invalid register");
                        exit(0);
                    }
                    fReg.valCValid = 1;
                    result = computeResult(ptr, counter);
                    fReg.valC = result;
                    fReg.byte0 = ptr[PC+2];
                    fReg.byte1 = ptr[PC+3];
                    fReg.byte2 = ptr[PC+4];
                    fReg.byte3 = ptr[PC+5];
                    fReg.byte4 = ptr[PC+6];
                    fReg.byte5 = ptr[PC+7];
                    fReg.byte6 = ptr[PC+8];
                    fReg.byte7 = ptr[PC+9];
                    fReg.valP = PC + 10;  
                    fReg.instr = "rmmovq";
                    printRegS(&fReg);
                    PC += 10;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 10, real_read_len - counter);
                    exit(0);
                }
                break;

                case 5:
                counter = PC;
                if(real_read_len - counter >= 10){
                    fReg.PC = PC;
                    fReg.icode = icode; 
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 1; 
                    fReg.rA = (ptr[PC+1] >> 4) & 0x0F;  
                    fReg.rB = ptr[PC+1] & 0x0F;
                    if(fReg.rA == 0xf || fReg.rB == 0xf){
                        printf("Invalid register");
                        exit(0);
                    }
                    fReg.valCValid = 1; 
                    counter = PC;
                    result = computeResult(ptr, counter);
                    fReg.valC = result;
                    fReg.byte0 = ptr[PC+2]; 
                    fReg.byte1 = ptr[PC+3]; 
                    fReg.byte2 = ptr[PC+4]; 
                    fReg.byte3 = ptr[PC+5];
                    fReg.byte4 = ptr[PC+6]; 
                    fReg.byte5 = ptr[PC+7];
                    fReg.byte6 = ptr[PC+8]; 
                    fReg.byte7 = ptr[PC+9];
                    fReg.valP = PC + 10;  
                    fReg.instr = "mrmovq";
                    printRegS(&fReg);
                    PC += 10;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 10, real_read_len - counter);
                    exit(0);
                }
                break;

                case 6:
                counter = PC;
                if(real_read_len - counter >= 2){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.regsValid = 1;
                    fReg.rA = (ptr[PC+1] >> 4) & 0x0F;
                    fReg.rB = ptr[PC+1] & 0x0F;
                    if(fReg.rA == 0xf || fReg.rB == 0xf){
                        printf("Invalid register");
                        exit(0);
                    }
                    fReg.valCValid = 0;
                    fReg.valP = PC + 2;
                    fReg.ifun = ptr[PC] & 0x0F;
                    switch(fReg.ifun){
                        case 0:
                        fReg.instr = "addq";
                        printRegS(&fReg);
                        break;
                        case 1:
                        fReg.instr = "subq";
                        printRegS(&fReg);
                        break;
                        case 2:
                        fReg.instr = "andq";
                        printRegS(&fReg);
                        break;
                        case 3:
                        fReg.instr = "xorq";
                        printRegS(&fReg);
                        break;
                        case 4:
                        fReg.instr = "mulq";
                        printRegS(&fReg);
                        break;
                        case 5:
                        fReg.instr = "divq";
                        printRegS(&fReg);
                        break;
                        case 6:
                        fReg.instr = "modq";
                        printRegS(&fReg);
                        break;
                        default:
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    PC += 2;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 2, real_read_len - counter);
                    exit(0);
                }
                break;

                case 7:
                counter = PC;
                if(real_read_len - counter >= 9){
                    fReg.PC = PC;
                    fReg.icode = icode; 
                    fReg.ifun = ptr[PC] & 0x0F;
                    fReg.regsValid = 0; 
                    fReg.valCValid = 1; 
                    counter = PC;
                    result = computeResult(ptr, counter);
                    fReg.valC = result;
                    fReg.byte0 = ptr[PC+2]; 
                    fReg.byte1 = ptr[PC+3]; 
                    fReg.byte2 = ptr[PC+4]; 
                    fReg.byte3 = ptr[PC+5];
                    fReg.byte4 = ptr[PC+6]; 
                    fReg.byte5 = ptr[PC+7];
                    fReg.byte6 = ptr[PC+8]; 
                    fReg.byte7 = ptr[PC+9];
                    fReg.valP = PC + 9;
                    switch(fReg.ifun){
                        case 0:
                        fReg.instr = "jmp";
                        printRegS(&fReg);
                        break;
                        case 1:
                        fReg.instr = "jle";
                        printRegS(&fReg);
                        break;
                        case 2:
                        fReg.instr = "jl";
                        printRegS(&fReg);
                        break;
                        case 3:
                        fReg.instr = "je";
                        printRegS(&fReg);
                        break;
                        case 4:
                        fReg.instr = "jne";
                        printRegS(&fReg);
                        break;
                        case 5:
                        fReg.instr = "jge";
                        printRegS(&fReg);
                        break;
                        case 6:
                        fReg.instr = "jg";
                        printRegS(&fReg);
                        break;
                        default:
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    PC += 9;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 9, real_read_len - counter);
                    exit(0);
                }
                break;

                case 8:
                counter = PC;
                if(real_read_len - counter >= 9){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 0;
                    fReg.valCValid = 1;
                    counter = PC;
                    result = computeResult(ptr, counter);
                    fReg.valC = result;
                    fReg.byte0 = ptr[PC+2];
                    fReg.byte1 = ptr[PC+3];
                    fReg.byte2 = ptr[PC+4];
                    fReg.byte3 = ptr[PC+5];
                    fReg.byte4 = ptr[PC+6];
                    fReg.byte5 = ptr[PC+7];
                    fReg.byte6 = ptr[PC+8];
                    fReg.byte7 = ptr[PC+9];
                    fReg.valP = PC + 9;
                    fReg.instr = "call";
                    printRegS(&fReg);
                    PC += 9;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 9, real_read_len - counter);
                    exit(0);
                }
                break;

                case 9:
                counter = PC;
                if(real_read_len - counter >= 1){
                    fReg.PC = PC;
                    fReg.icode = icode;
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 0;
                    fReg.valCValid = 0;
                    fReg.valP = PC + 1;
                    fReg.instr = "ret";
                    printRegS(&fReg);
                    PC += 1;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 1, real_read_len - counter);
                    exit(0);
                }
                break;

                case 0xA:
                counter = PC;
                if(real_read_len - counter >= 2){
                    fReg.PC = PC;
                    fReg.icode = icode; 
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 1;
                    fReg.rA = (ptr[PC+1] >> 4) & 0x0F;
                    fReg.rB = ptr[PC+1] & 0x0F;
                    if(fReg.rB != 0xf){
                        printf("Invalid register");
                        exit(0);
                    }
                    fReg.valCValid = 0;
                    fReg.valP = PC + 2;
                    fReg.instr = "pushq";
                    printRegS(&fReg);
                    PC += 2;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 2, real_read_len - counter);
                    exit(0);
                }
                break;

                case 0xB:
                counter = PC;
                if(real_read_len - counter >= 2){
                    fReg.PC = PC;
                    fReg.icode = icode; 
                    fReg.ifun = ptr[PC] & 0x0F;
                    if(fReg.ifun != 0){
                        printf("Invalid function code 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                        exit(0);
                    }
                    fReg.regsValid = 1; 
                    fReg.rA = (ptr[PC+1] >> 4) & 0x0F;
                    fReg.rB = ptr[PC+1] & 0x0F;
                    if(fReg.rB != 0xf){
                        printf("Invalid register");
                        exit(0);
                    }
                    fReg.valCValid = 0; 
                    fReg.valP = PC + 2;  
                    fReg.instr = "popq";
                    printRegS(&fReg);
                    PC += 2;
                }
                else{
                    printRegS(&fReg);
                    printf("Memory access error at 0x%016llX\n, required 0x%d bytes, read 0x%ld bytes.", PC, 2, real_read_len - counter);
                    exit(0);
                }
                break;

                default:
                printf("Invalid opcode 0x%x at 0x%016llX\n", ptr[PC] & 0xFF, PC);
                exit(0);
            }
            if(PC == real_read_len){
                printf("Normal termination.\n");
                exit(0);
            }
        }
    }

    // EXAMPLESON line
// #define EXAMPLESON 1
#ifdef  EXAMPLESON
    // The next few lines are examples of various types of output. In the
    // comments is an instruction, the address it is at and the associated
    // binary code that would be found in the object code file at that
    // address (offset). Your program will read that binary data and then
    // pull it appart just like the fetch stage. Once everything has been
    // pulled apart then a call to printRegS is made to have the output printed.
    // Read the comments in printInternalReg.h for what the various fields of
    // the structure represent. Note: Do not fill in fields int the structure
    // that aren't used by that instruction.

    /*************************************************
       irmovq $1, %rsi   0x008: 30f60100000000000000
    ***************************************************/

    fReg.PC = 8; fReg.icode = 3; fReg.ifun = 0;
    fReg.regsValid = 1; fReg.rA = 15;  fReg.rB = 6;
    fReg.valCValid = 1; fReg.valC = 1;
    fReg.byte0 = 1;  fReg.byte1 = 0; fReg.byte2 = 0; fReg.byte3 = 0;
    fReg.byte4 = 0;  fReg.byte5 = 0; fReg.byte6 = 0; fReg.byte7 = 0;
    fReg.valP = 8 + 10;  fReg.instr = "irmovq";

    printRegS(&fReg);

      /*************************************************
       je target   x034: 733f00000000000000     Note target is a label

       ***************************************************/

    fReg.PC = 0x34; fReg.icode = 7; fReg.ifun = 3;
    fReg.regsValid = 0;
    fReg.valCValid = 1; fReg.valC = 0x3f;
    fReg.byte0 = 0x3f;  fReg.byte1 = 0; fReg.byte2 = 0; fReg.byte3 = 0;
    fReg.byte4 = 0;  fReg.byte5 = 0; fReg.byte6 = 0; fReg.byte7 = 0;
    fReg.valP = 0x34 + 9;  fReg.instr = "je";

    printRegS(&fReg);

      /*************************************************
       nop  x03d: 10
       ***************************************************/

    fReg.PC = 0x3d; fReg.icode = 1; fReg.ifun = 0;
    fReg.regsValid = 0;
    fReg.valCValid = 0;
    fReg.valP = 0x3d + 1;  fReg.instr = "nop";

    printRegS(&fReg);

      /*************************************************
       addq %rsi,%rdx  0x03f: 6062
       ***************************************************/

    fReg.PC = 0x3f; fReg.icode = 6; fReg.ifun = 0;
    fReg.regsValid = 1; fReg.rA = 6; fReg.rB = 2;
    fReg.valCValid = 0;
    fReg.valP = 0x3f + 2;  fReg.instr = "add";

    printRegS(&fReg);
#endif
    return SUCCESS;
}
