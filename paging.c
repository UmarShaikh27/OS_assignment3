#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h> //for uint ints

typedef struct{
    int frame_num;
    int valid;
} PTE;

typedef struct
{
    PTE* entries;
    int num_pages;
} pageTable;

typedef struct{
    int procid;
    int procsize;
    char* filename;
    pageTable* pagetable;
} PCB;


void load_process(char* filename){
    //OPEN THE FILE
    FILE *procfile = fopen(filename, "rb"); //rb means reading file in binary mode
    if(!procfile){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    //FILE CONTENTS
    uint8_t procid;
    uint16_t codesize;
    uint8_t *code_seg = malloc(sizeof(uint8_t)*codesize);
    uint16_t datasize;
    uint8_t *data_seg = malloc(sizeof(uint8_t)*datasize);
    uint8_t end;

    //RETRIEVING FILE CONTENT
    fread(&procid,sizeof(uint8_t), 1, procfile);

    fread(&codesize,sizeof(uint16_t), 1, procfile);
    codesize = ntohs(codesize);
    fread(code_seg,sizeof(uint8_t), codesize, procfile);

    fread(&datasize,sizeof(uint16_t), 1, procfile);
    datasize = ntohs(datasize);
    fread(data_seg,sizeof(uint8_t), datasize, procfile);

    fread(&end,sizeof(uint8_t), 1, procfile);

    //ERROR HANDLING FOR WHEN END MARKER NOT FOUND
    if(end != 0xFF){
        //THROW ERROR AND FREE MEMORY
        printf("Error : end marker not found in process: %u\n", (unsigned int)procid);
        free(code_seg);
        free(data_seg);
        fclose(procfile);
        return;
    }

    //FILL CONTENT IN PCB
    // procblock->procid = procid;
    // procblock->procsize = codesize+datasize;

    //PRINT CHECKS
    printf("Process id: %u\n", (unsigned int)procid);
    printf("Code size: %u\n" ,(unsigned int)codesize);
    printf("Data size id: %u\n",(unsigned int)datasize);
    // printf("Code segment id: %u\n");
    // printf("Data segment id: %u\n");


    // pageTable* pt
    

    

}


int main(int argc, char* argv[]){
    if(argc<5){
        printf("Usage: %s <physical mem size> <address space size> <page size> <list of processes>", argv[0]);
    }
    int mem_size = atoi(argv[1]);
    int log_space_bits = atoi(argv[2]);
    int page_size = atoi(argv[3]);
    int num_proc = argc-4; //number of processes

    PCB* ready_queue  = malloc(num_proc*sizeof(PCB)); //list for PCBs

    // for(int i= 4; i < argc; i++){
        //iterate on processes

    // }

    printf("FILE PATH: ", argv[4]);
    load_process(argv[4]);
    
    return 0;
}