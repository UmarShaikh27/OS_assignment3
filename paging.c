#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h> //for uint ints
#include <string.h>

//PAGE TABLE ENTRY
typedef struct{
    int frame_num;
    int valid;
} PTE;

//PROCESS CONTROL BLOCK
typedef struct{
    int procid;
    int procsize;
    char* filename;
    PTE* pagetable;
} PCB;

//helper function to find the first available frame from the list of physical frames
int findframe(int *framelist,int numframes){
    for(int i=0;i<numframes;i++){
        if(framelist[i] == 1){
            return i; //return frame number when found
        }
    }
    return -1; //ERROR : all frames occupied
}

//function to create page table and allocate frames
void load_process(char* filename, int pagesize, PCB* proc, int framelist[], int numframes){

    //OPEN THE FILE
    FILE *procfile = fopen(filename, "rb"); //rb means reading file in binary mode
    if(!procfile){
        printf("Error opening the file : %s\n",filename);
        return; // dont proceed with loading this process 
    }

    //FILE CONTENTS
    uint8_t procid;
    uint16_t codesize;
    uint8_t *code_seg = malloc(sizeof(uint8_t)*codesize);
    uint16_t datasize;
    uint8_t *data_seg = malloc(sizeof(uint8_t)*datasize);
    uint8_t end;

    //RETRIEVING FILE CONTENTS
    fread(&procid,sizeof(uint8_t), 1, procfile);
    fread(&codesize,sizeof(uint16_t), 1, procfile);
    codesize = ntohs(codesize); //conversion from big endian
    fread(code_seg,sizeof(uint8_t), codesize, procfile);

    fread(&datasize,sizeof(uint16_t), 1, procfile);
    datasize = ntohs(datasize); //conversion from big endian
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
    proc->pagetable = malloc(sizeof(PTE)*numpages);

    for(int i=0; i<numpages; i++){
        int frame = findframe(framelist,numframes);
        if(frame == -1){
            printf("Error: No more physical frames left");
            break;
        }
        proc->pagetable[i].frame_num = frame;
        proc->pagetable[i].valid=1;
        framelist[frame] = 0;
    }

    //PRINT CHECKS
    printf("Loaded process => Process id: %u\n", (unsigned int)procid);
    printf("-----Number of pages: %d\n", numpages);
    printf("-----Process Size: %u\n" ,(unsigned int)(codesize+datasize));
    // printf("Process size : %u\n",(unsigned int)datasize);

    //FREE MEMORY
    free(code_seg);
    free(data_seg);
    
}

void displayFreeFrames(int *framelist, int numFrames) {
    printf("========FREE FRAME LIST========\n");
    printf("Free Frames: \n[");
    for (int i=0; i< numFrames; i++){
        if(framelist[i] == 1) printf("%d ", i);
    }
    printf("]\n\n");
}

//FUNCTION TO RETURN THE TOTAL FRAGMENTATION OF ALL PROCESSES COMBINED
int calculateFragmentation(PCB* ready_queue, int num_proc, int page_size){
    printf("========FRAGMENTATION========\n");
    int totalfrag =0;
    for(int i =0; i<num_proc ;i++){
        int lastpage = ready_queue[i].procsize % page_size; //modulo of size of process with size of page will give size of last page
        if(lastpage>0){
            totalfrag += page_size-lastpage; //pagesize - lastpagesize will give the fragmentation for the process
        }
    }
    return totalfrag;
}

//FUNCTION TO PRINT ALL PAGE TABLE ENTRIES FOR EACH PROCESS
void memdump(PCB* ready_queue, int page_size,int numproc){
    printf("========MEMORY DUMP========\n");
    for(int i=0; i<numproc;i++){ //loop for processes
        printf("---PROCESS %d---\n", i+1);
        int numpages = (ready_queue[i].procsize+page_size-1)/page_size; 
        printf("Number of Pages = %d\n", numpages);
        for(int j=0;j<numpages;j++){ //loop for pages
            printf("VPN %d | PFN %d\n",j, ready_queue[i].pagetable[j].frame_num);
        }
    }
    printf("\n");//break
}

//HELPER TO CHECK WHETHER THE FILE IS .proc OR NOT
int has_proc_extension(const char *filename) {
    const char *ext = strrchr(filename, '.'); // Find the last '.' in the string
    return (ext != NULL && strcmp(ext, ".proc") == 0);
}

int main(int argc, char* argv[]){
    //-----------ERROR HANDLING-----------
    if(argc<5){
        printf("USAGE: make run ARGS=\"<physical mem size> <address space size> <page size> 'path1' 'path2'...'pathn'\"\n");
        printf("OR\n");
        printf("Usage: %s <physical mem size> <address space size> <page size> <list of processes>\n", argv[0]);
        exit(EXIT_FAILURE);//exit function
    }
    for (int i = 4; i < argc; i++) {
        if (!has_proc_extension(argv[i])) {
            printf("Error: Argument '%s' is not a .proc file.\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    //--------RETRIEVE GIVEN DATA------------
    int mem_size = atoi(argv[1]);
    int log_space_bits = atoi(argv[2]);
    int page_size = atoi(argv[3]);

    //INITIALIZING READY QUEUE
    int num_proc = argc-4; //number of processes
    PCB* ready_queue  = malloc(num_proc*sizeof(PCB)); //list for PCBs
    int numframes = mem_size/page_size; //number of frames
    

    //STORING PHYSICAL FRAMES
    int framelist[numframes];
    for(int i=0; i<numframes; i++){
        framelist[i]=1; //all frames assigned 1 initially meaning all of them are free
    }

    //LOADING PROCESS
    printf("========PROCESSES LOADING========\n");
    for(int i = 0; i+4<argc; i++){
        printf("Loading process %d...\n",i+1);
        load_process(argv[i+4], page_size, &ready_queue[i], framelist, numframes); //loadprocess called on every file 
    }
    printf("\n"); //BREAK

    //DISPLAYING OUTPUTS
    memdump(ready_queue,page_size, num_proc); //page table prints
    displayFreeFrames(framelist, numframes); //displaying free frames remaining
    int frag = calculateFragmentation(ready_queue, num_proc, page_size); //displaying fragmentation
    printf("Total Fragmentation is: %d\n", frag);

    //FREE DYNAMIC MEMORY ALLOCATION
    //page tables
    for (int i = 0; i < num_proc; i++) {
        free(ready_queue[i].pagetable);
    }
    //ready queue
    free(ready_queue);

    
    return 0;
}