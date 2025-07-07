#define SYSCALL_FAILED(POINTER) (((long int) POINTER) == -1)
//#define BYTES_IN_ARCHITECTURE (4) //4 if we are in a 32 bit machine, if in 64 bit then it should be 8
#define EXTRA_DATA_AMMOUNT (4) //ammount of actual variables we would need to allocate

#define ARCH_SIZE (sizeof(void*)) //getting tired of writing sizeof(void*) everywhere, made a define for easier use


enum MemoryStatus {
    FREE = 0,
    ALLOCATED = 1
};

typedef unsigned long size_t; //FIXME: delete this line! its only for my mac
void* sbrk(size_t size); //FIXME: delete this line! its only for my mac


inline bool _isFlag(void* block, MemoryStatus flag){
    if (!block){
        //make it safe to call on nullptr, similar behaviour to dev/null
        //as per the tutorials or lectures or whatever
        return true;
    }
    //go 2 'objects' below the pointer
    void* address_2_below = (block-2*ARCH_SIZE);

    size_t value_at_addres_2_below = *((size_t*)address_2_below);
    

    //check if last bit is 1 or 0
    return ((value_at_addres_2_below&flag)==flag);
}

inline bool isAllocated(void* block){
    return _isFlag(block, ALLOCATED);
}

#if 0 // i dont think we need this function
inline bool isFree(void* block){
    return _isFlag(block, FREE);
}
#endif

#if 0 //didnt find a good way to do it
inline void _apply_to_flag(void* block, MemoryStatus flag){
        if (!block){
        //make it safe to call on nullptr, similar behaviour to dev/null
        //as per the tutorials or lectures or whatever
        return;
    }

    //go 2 'objects' below the pointer
    size_t* address_2_below = (size_t*) (block-2*ARCH_SIZE);

    ((*address_2_below)&=flag)|=flag;
}
#endif

inline void markAllocated(void* block){
    if (!block){
        //make it safe to call on nullptr, similar behaviour to dev/null
        //as per the tutorials or lectures or whatever
        return;
    }

    //go 2 'objects' below the pointer
    size_t* address_2_below = (size_t*) (block-2*ARCH_SIZE);

    (*address_2_below)|=ALLOCATED;
}

inline void markFree(void* block){
    if (!block){
        //make it safe to call on nullptr, similar behaviour to dev/null
        //as per the tutorials or lectures or whatever
        return;
    }

    //go 2 'objects' below the pointer
    size_t* address_2_below = (size_t*) (block-2*ARCH_SIZE);

    //easiest way i found to make the first bit 0 without using a predefined register size,
    //just make sure the first bit is 1 and then xor it with 1.
    ((*address_2_below)|=ALLOCATED)^=ALLOCATED;
}

inline size_t getBlockSize(void* block){
    if (!block){
        //make it safe to call on nullptr, similar behaviour to dev/null
        //as per the tutorials or lectures or whatever
        return 0;
    }

    //go 2 'objects' below the pointer
    size_t* address_2_below = (size_t*) (block-2*ARCH_SIZE);

    size_t size = *address_2_below;

    //easiest way i found to make the first bit 0 without using a predefined register size,
    //just make sure the first bit is 1 and then xor it with 1.
    ((size)|=ALLOCATED)^=ALLOCATED;

    return size;
}


/**
● Tries to allocate ‘size’ bytes.
● Return value:
i. Success –a pointer to the first allocated byte within the allocated block.
 ii. Failure –
    a. If ‘size’ is 0 returns NULL.
    b. If ‘size’ is more than 10**8, return NULL.
    c. If sbrk fails, return NULL.  

Notes:
    ● size_t is a typedef to unsigned int in 32-bit architectures, and to unsigned long long in 64-bit
      architectures. This means that trying to insert a negative value will result in compiler warning.
    ● You do not need to implement free(), calloc() or realloc() for this section.
      
    Discussion: Before proceeding, try discussing the current implementation with your partner. What’s
    wrong with it? What’s missing? Are we handling fragmentation? What would you do differently?
*/
void* smalloc(size_t size){
    /**
    Heap Overview:

      XX--other data--XX
        block size (long) //no allocation flag here!
              |
    next avalible block (long) //maintained only for 'free' blocks, points to the addres that was returned from the malloc call (not actual meta_block_start of block)
              |
              |
          'payload' //('long aligned' blocks of size of user request)
              |
              | <- returned *ptr (actual start of free space)
    prev avalible block (long)
              |
        block size (long) //(last bit of size would be the flag for malloced/ free, as per the ATAM lecture) (1 = allocated, 0 = free)
      XX--other data--XX

     */

    if (size <= 0 || size > 100000000){
        return nullptr;
    }

    void* meta_block_start = sbrk(0);

    if (SYSCALL_FAILED(meta_block_start)){
        return nullptr;
    }

    size_t alligned_size = ((size)+(size%ARCH_SIZE)); //allign size

    size_t actual_size =  alligned_size + EXTRA_DATA_AMMOUNT*ARCH_SIZE;

    void* one_after_meta_block_end = sbrk(actual_size);

    if (SYSCALL_FAILED(one_after_meta_block_end)){
        return nullptr;
    }

    void* meta_block_end = (one_after_meta_block_end - ARCH_SIZE);

    //add actual size metadata to the edges of the blocks
    *((size_t*)meta_block_start) = actual_size;
    *((size_t*)meta_block_end) = actual_size;

    size_t* pointer_to_prev_block = (((size_t*)meta_block_start) + 1*ARCH_SIZE);
    size_t* pointer_to_next_block = (((size_t*)meta_block_end) - 1*ARCH_SIZE);

    //nulify pointers to avoid garbage causing problems;
    *pointer_to_prev_block = 0;
    *pointer_to_next_block = 0;


    void* payload_start = (meta_block_start + 2*ARCH_SIZE);

    return payload_start;

}
