#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h> //for uint ints
#include <string.h>

typedef struct{
    int frame_num;
    int valid;
} PTE;

// typedef struct
// {
//     PTE* entries;
//     int num_pages;
// } pageTable;

typedef struct{
    int procid;
    int procsize;
    char* filename;
    PTE* pagetable;
} PCB;

int findframe(int *framelist,int numframes){
    for(int i=0;i<numframes;i++){
        if(framelist[i] == 1){
            return i;
        }
    }
    return -1; //ERROR
}

void load_process(char* filename, int pagesize, PCB* proc, int framelist[], int numframes){
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
    proc->procid = procid;
    proc->procsize = codesize+datasize;

    int numpages = (proc->procsize+pagesize-1)/pagesize; //pagesize-1 added to ensure partial pages are also catered
    printf("Number of pages: %d\n", numpages);
    proc->pagetable = malloc(sizeof(PTE)*numpages);
    printf("Page table allocated\n");

    for(int i=0; i<numpages; i++){
        int frame = findframe(framelist,numframes);
        printf("Found frame = %d\n",frame);
        if(frame == -1){
            printf("Error: No more physical frames left");
            break;
        }
        proc->pagetable[i].frame_num = frame;
        // printf("Check print\n");
        proc->pagetable[i].valid=1;
        //  printf("Check print\n");
        framelist[frame] = 0;
        // printf("Check print\n");
    }

    //PRINT CHECKS
    printf("Process id: %u\n", (unsigned int)procid);
    printf("Code size: %u\n" ,(unsigned int)codesize);
    printf("Process size : %u\n",(unsigned int)datasize);
    
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
    int numframes = mem_size/page_size; 
    // int numframes = 120; 

    printf("Number of physical frames = %d\n",numframes);
    // int* framelist = malloc(sizeof(int)*numframes);
    int framelist[numframes];
    for(int i=0; i<numframes; i++){
        framelist[i]=1;
    }

     
    // for(int i= 4; i < argc; i++){
        //iterate on processes

    // }

    // printf("FILE PATH: %s\n", argv[4]);
    load_process(argv[4], page_size, &ready_queue[0], framelist, numframes);
    
    return 0;
}