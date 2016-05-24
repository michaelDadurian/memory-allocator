/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfmm.h"


/**
 * You should store the head of your free list in this variable.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
sf_free_header* freelist_head = NULL;




#define PAGE_SIZE 4096

#define WORD 4 
#define DOUBLE 8
#define ALIGNMENT 8
#define ROUNDUP(n) ALIGNMENT * (size_t)((n + (ALIGNMENT) + (ALIGNMENT-1)) / ALIGNMENT)
#define MAX(x, y) ((x) > (y) ? (x):(y)) 
#define MIN 16

//asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);



//space
int space = PAGE_SIZE;



//takes number of bytes as input, returns a pointer to a block of memory of that size
//sbrk(0) returns a pointer to the current top of the heap
//sbrk(foo) increments heap size by foo and returns a pointer to the previous top of the heap
void* sf_malloc(size_t size) {

    size_t adjustedSize;


    if (size == 0){
        return NULL;
    }

    if (freelist_head == NULL){

        sf_header * start = sf_sbrk(8); //8 byte padding
        start->alloc = 1;
        start->block_size = 8;
        start->requested_size=0;

        freelist_head = sf_sbrk(PAGE_SIZE);

        

        sf_header * header = start + 8;
        printf("%p\n", header);
        header -> alloc = 0;
        header -> requested_size = 0;
        header -> block_size = 4088;

        sf_footer * footer = sf_sbrk(0) - 8;
        footer -> alloc = 0;
        footer -> block_size = 4096;

    
    }


    void *a = freelist_head; //current top of heap
    

    if((long)a%16 == 0){
        a += 8;
        space -= 8;
    }
        

    
    adjustedSize = MAX(ROUNDUP(size) - 8, MIN);
    

    sf_header *header = a;
    header -> alloc = 1;
    header -> block_size = (adjustedSize + 16) >> 4;
    header -> requested_size = size;

    

    sf_footer *footer = a + adjustedSize + 8;
    footer -> alloc = 1;
    footer -> block_size = (adjustedSize + 16) >> 4;

    //subtract 32 from total available space when malloc is called
    space -= 32;
    sf_free_header *newFreeBlock = a + adjustedSize + 16;
    newFreeBlock -> header.block_size = space >> 4;
    newFreeBlock -> header.alloc = 0;
    newFreeBlock -> header.requested_size = 0;

    sf_footer *endOfList = sf_sbrk(0) - 8;
    endOfList -> block_size = space >> 4;
    

    /*endOfList -> block_size = space<< 4;
    endOfList -> alloc = 0;*/


    freelist_head = (a + adjustedSize + 16); //payload + header + footer
    
    
    return a+8;
}


//0x7f8fe6542118
//0x7f8fe6543010

//0x7f911b2dc118
//0x7f911b2dd008



/*
struct __attribute__((__packed__)) sf_header{
    uint64_t alloc : ALLOC_SIZE_BITS;
    uint64_t block_size : BLOCK_SIZE_BITS;
    uint64_t requested_size : REQST_SIZE_BITS;
};
typedef struct sf_header sf_header;

struct __attribute__((__packed__)) sf_free_header {
    sf_header header;
    struct sf_free_header *next;
    struct sf_free_header *prev;
};
typedef struct sf_free_header sf_free_header;

struct __attribute__((__packed__)) sf_footer {
    uint64_t alloc : ALLOC_SIZE_BITS;
    uint64_t block_size : BLOCK_SIZE_BITS;
   */

void sf_free(void *ptr) {
    if (ptr == NULL)
        return;
    //case 1, no coalescing
    
        //temp holds pointer to next free head

    sf_free_header *temp = freelist_head;
        //address of payload - 8 = header
    sf_free_header *headerOfPtr = ptr - 8; 
        
        
        //shift left 4 to get rid of alloc field
    int blockSize = headerOfPtr -> header.block_size << 4;
        
    sf_footer *footerOfPtr = ptr + blockSize -16;
        

        //set footer and header to free and have same block size
    footerOfPtr -> alloc = 0;
    footerOfPtr -> block_size = blockSize>>4;
        
        
    headerOfPtr -> header.alloc = 0;
    headerOfPtr -> header.requested_size= 0;

        //adjust pointers
    freelist_head = headerOfPtr;
    freelist_head -> header = headerOfPtr -> header;
    headerOfPtr -> next = temp;
    temp -> prev = headerOfPtr;
    

    //}
        //0x7f28440de138


        
    
}

/**
 * Resizes the memory pointed to by ptr to be size bytes.
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 * @return If successful, the pointer to a valid region
 * of memory to use is returned, else the value NULL is
 * returned and the ERRNO is set accordingly.
 *
 * A realloc call with a size of zero should return NULL
 * and set the ERRNO accordingly.
 */

 //If the realloc size is smaller than the original size, 
 //resize the block in place (change header values, move footer)

 //If the realloc size is larger than the original size, then allocate a new block, 
 //copy the payload of the old block to the new space, and then free the old block
void* sf_realloc(void *ptr, size_t size) {

    void * validRegion;
    size_t adjustedSize = MAX(ROUNDUP(size) - 8, MIN);
    
    sf_header * ptrHeader = ptr - 8;
    //sf_footer * ptrFooter = ptr + adjustedSize + 8;
    int originalSize = (*(int*)ptrHeader & ~0x7);

    //if ptr is null, just malloc
    if (ptr == NULL)
        return sf_malloc(size);
    //if size is 0, then just free and return NULL
    if (size == 0){
        sf_free(ptr);
        return NULL;
    }

    //if the sizes are the same just return ptr
    if (adjustedSize == originalSize)
        return ptr;

    //#define PUT(p, val)  (*(int *)(p) = (val))
    //#define PACK(size, alloc)  ((size) | (alloc))
    if (originalSize < adjustedSize){
        validRegion = sf_malloc(size);
        memcpy(validRegion, ptr, originalSize);
        sf_free(ptr);
    } else{
        validRegion = ptr;
    }

    return validRegion;


}

void* sf_calloc(size_t nmemb, size_t size) {
    return NULL;
}