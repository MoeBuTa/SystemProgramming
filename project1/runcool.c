//  CITS2002 Project 1 2021
//  Name(s):             Wenxiao Zhang
//  Student number(s):   22792191
//  compile with:  cc -std=c11 -Wall -Werror -o runcool runcool.c

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

//  THE STACK-BASED MACHINE HAS 2^16 (= 65,536) WORDS OF MAIN MEMORY
#define N_MAIN_MEMORY_WORDS (1 << 16)

//  EACH WORD OF MEMORY CAN STORE A 16-bit UNSIGNED ADDRESS (0 to 65535)
#define AWORD uint16_t
//  OR STORE A 16-bit SIGNED INTEGER (-32,768 to 32,767)
#define IWORD int16_t

//  THE ARRAY OF 65,536 WORDS OF MAIN MEMORY
AWORD main_memory[N_MAIN_MEMORY_WORDS];

//  THE SMALL-BUT-FAST CACHE HAS 32 WORDS OF MEMORY
#define N_CACHE_WORDS 32

//  see:  https://teaching.csse.uwa.edu.au/units/CITS2002/projects/coolinstructions.php
enum INSTRUCTION
{
    I_HALT = 0,
    I_NOP,
    I_ADD,
    I_SUB,
    I_MULT,
    I_DIV,
    I_CALL,
    I_RETURN,
    I_JMP,
    I_JEQ,
    I_PRINTI,
    I_PRINTS,
    I_PUSHC,
    I_PUSHA,
    I_PUSHR,
    I_POPA,
    I_POPR
};

//  USE VALUES OF enum INSTRUCTION TO INDEX THE INSTRUCTION_name[] ARRAY
const char *INSTRUCTION_name[] = {
    "halt",
    "nop",
    "add",
    "sub",
    "mult",
    "div",
    "call",
    "return",
    "jmp",
    "jeq",
    "printi",
    "prints",
    "pushc",
    "pusha",
    "pushr",
    "popa",
    "popr"};
//  ----  IT IS SAFE TO MODIFY ANYTHING BELOW THIS LINE  --------------

//  THE STATISTICS TO BE ACCUMULATED AND REPORTED
int n_main_memory_reads = 0;
int n_main_memory_writes = 0;
int n_cache_hits = 0;
int n_cache_misses = 0;

//PROGRAM SIZE
int program;

//CACHE MEMORY
struct cache
{
    bool dirty; //dirty tag, set true if dirty
    AWORD address;
    AWORD data;
} cache_memory[N_CACHE_WORDS];

// CACHE MEMORY
// AWORD cache_memory[N_CACHE_WORDS];

// INITIALIZE CACHE
void initialize_cache()
{

    for (int i = 0; i < N_CACHE_WORDS; i++)
    {
        cache_memory[i].dirty = true;
    }
}

void report_statistics(int result)
{
    printf("@number-of-main-memory-reads\t%i\n", n_main_memory_reads);
    printf("@number-of-main-memory-writes\t%i\n", n_main_memory_writes);
    printf("@number-of-cache-memory-hits\t%i\n", n_cache_hits);
    printf("@number-of-cache-memory-misses\t%i\n", n_cache_misses);
    // printf("@number-of-main-memory-reads-(fast-jeq)\t%i\n", n_main_memory_reads - n_main_memory_reads_fast_jeq);
    printf("@exit(%i)\n", result);
}

//  -------------------------------------------------------------------

//  EVEN THOUGH main_memory[] IS AN ARRAY OF WORDS, IT SHOULD NOT BE ACCESSED DIRECTLY.
//  INSTEAD, USE THESE FUNCTIONS read_memory() and write_memory()
//
//  THIS WILL MAKE THINGS EASIER WHEN WHEN EXTENDING THE CODE TO
//  SUPPORT CACHE MEMORY

void write_back(struct cache cache)
{
    //check if write-back is needed
    if (cache.dirty && cache.address > program && main_memory[cache.address] != cache.data)
    {
        ++n_main_memory_writes;
        main_memory[cache.address] = cache.data;
    }
}

AWORD read_memory(int address)
{
    // index: log2(32) = 5   11111 -> 1F
    AWORD index = address & 0x1F;
    if (cache_memory[index].address == address && address != 0)
    {
        ++n_cache_hits;
        return cache_memory[index].data;
    }

    // cache miss
    ++n_cache_misses;
    ++n_main_memory_reads;
    IWORD value = main_memory[address];

    //write back
    write_back(cache_memory[index]);

    cache_memory[index].dirty = false;     //not dirty
    cache_memory[index].address = address; // address tag
    cache_memory[index].data = value;      // data
    return value;
}

void write_memory(AWORD address, AWORD value)
{
    AWORD index = address & 0x1F;
    if (cache_memory[index].address != address || cache_memory[index].data != value)
    {
        write_back(cache_memory[index]);
        cache_memory[index].dirty = true; // dirty
    }
    cache_memory[index].address = address; // address tag
    cache_memory[index].data = value;      // data
}

//  -------------------------------------------------------------------

//  EXECUTE THE INSTRUCTIONS IN main_memory[]
int execute_stackmachine(void)
{
    //  THE 3 ON-CPU CONTROL REGISTERS:
    int PC = 0;                   // 1st instruction is at address=0
    int SP = N_MAIN_MEMORY_WORDS; // initialised to top-of-stack
    int FP = 0;                   // frame pointer

    //  REMOVE THE FOLLOWING LINE ONCE YOU ACTUALLY NEED TO USE FP

    while (true)
    {
        //  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction = read_memory(PC++);
        IWORD value;
        IWORD address;
        IWORD value1;
        IWORD value2;
        IWORD offset;

        if (instruction == I_HALT)
        {
            break;
        }

        switch (instruction)
        {
        case I_NOP:
            break;
        case I_ADD:
            value1 = read_memory(SP++);
            value2 = read_memory(SP);
            write_memory(SP, value1 + value2);
            break;
        case I_SUB:
            value1 = read_memory(SP++);
            value2 = read_memory(SP);
            write_memory(SP, value2 - value1);
            break;
        case I_MULT:
            value1 = read_memory(SP++);
            value2 = read_memory(SP);
            write_memory(SP, value1 * value2);
            break;
        case I_DIV:
            value1 = read_memory(SP++);
            value2 = read_memory(SP);
            write_memory(SP, value2 / value1);
            break;
        case I_CALL:
            write_memory(--SP, PC);    // save function address onto the stack
            write_memory(--SP, FP);    // save previous FP onto the stack
            FP = SP;                   // copy SP into FP
            address = read_memory(PC); // function address
            PC = address;
            break;
        case I_RETURN:
            offset = read_memory(PC);     // read FP offset
            PC = read_memory(FP + 1) + 1; // reset PC(return address)
            value = read_memory(SP);      // read return value (TOS)
            SP = FP + offset;             //reset SP
            write_memory(SP, value);      // use FP offset to write the return value
            FP = read_memory(FP);         // reset FP
            break;
        case I_JMP:
            address = read_memory(PC); //read the address the flow of execution continues at
            PC = address;
            break;
        case I_JEQ:
            value = read_memory(SP++); // pop the value on the TOS
            address = read_memory(PC); //read the address the flow of execution continues at
            if (value == 0)
            {
                PC = address;
            }
            else
            {
                PC++;
            }
            break;
        case I_PRINTI:
            value = read_memory(SP++); // pop the value on the TOS
            printf("%i", value);
            break;
        case I_PRINTS:
            address = read_memory(PC++); //address in the data segment
            int word;
            char bytes[2];
            while (true)
            {
                word = read_memory(address++); // read 2-byte word in integer format
                bytes[0] = word & 0xFF;        // convert first byte to char
                bytes[1] = (word >> 8) & 0xFF; // convert second byte to char
                printf("%s", bytes);           // print the 2-byte 'word'
                if (bytes[0] == '\0' || bytes[1] == '\0')
                {
                    break;
                }
            }
            break;
        case I_PUSHC:
            value = read_memory(PC++);
            write_memory(--SP, value);
            break;
        case I_PUSHA:
            address = read_memory(PC++);  // read the address of the value to be pushed onto the stack.
            value = read_memory(address); // read the value by the address
            write_memory(--SP, value);
            break;
        case I_PUSHR:
            offset = read_memory(PC++); // read the FP offset into which the value on the TOS should be pushed onto the stack.
            value = read_memory(FP + offset);
            write_memory(--SP, value);
            break;
        case I_POPA:
            address = read_memory(PC++); // read the address into which the value on the TOS should be popped
            value = read_memory(SP++);   // pop TOS value
            write_memory(address, value);
            break;
        case I_POPR:
            offset = read_memory(PC++); // read the FP offset into which the value on the TOS should be popped
            value = read_memory(SP++);  // pop TOS value
            write_memory(FP + offset, value);
            break;
        }
    }

    //  THE RESULT OF EXECUTING THE INSTRUCTIONS IS FOUND ON THE TOP-OF-STACK
    return read_memory(SP);
}

//  -------------------------------------------------------------------

//  READ THE PROVIDED coolexe FILE INTO main_memory[]
void read_coolexe_file(char filename[])
{
    memset(main_memory, 0, sizeof main_memory); //  clear all memory

    //  READ CONTENTS OF coolexe FILE

    // ATTEMPT TO OPEN THE FILE FOR READ-ONLY ACCESS
    int fd = open(filename, O_RDONLY);
    // CHECK TO SEE IF FILE COULD BE OPENED
    if (fd == -1)
    {
        printf("cannot open '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    program = read(fd, main_memory, sizeof main_memory) / 2;
}

//  -------------------------------------------------------------------

int main(int argc, char *argv[])
{
    //  CHECK THE NUMBER OF ARGUMENTS
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s program.coolexe\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //  READ THE PROVIDED coolexe FILE INTO THE EMULATED MEMORY
    read_coolexe_file(argv[1]);

    // SET ALL CACHE BLOCK 'DIRTY'
    initialize_cache();

    //  EXECUTE THE INSTRUCTIONS FOUND IN main_memory[]
    int result = execute_stackmachine();

    report_statistics(result);

    return result; // or  exit(result);
}
