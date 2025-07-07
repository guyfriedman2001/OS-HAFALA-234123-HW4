typedef unsigned long size_t; //FIXME: delete this line! its only for my mac

/* typedef for clarity */
typedef void* payload_start;
typedef void* actual_block_start;

/* functions from the hw */
payload_start smalloc(size_t size);
payload_start scalloc(size_t num, size_t size);
void sfree(payload_start p);
payload_start srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();


/* our helper functions */
payload_start smalloc_helper_find_avalible(size_t size);
actual_block_start actually_allocate(size_t size);
inline void markFree(payload_start block);


payload_start smalloc(size_t size){
    /*
    ● Searches for a free block with at least ‘size’ bytes or allocates (sbrk()) one if none are
found.
● Return value:
i. Success – returns pointer to the first byte in the allocated block (excluding the meta-data of
course)
ii. Failure –
a. If size is 0 returns NULL.
b. If ‘size’ is more than 108, return NULL.
c. If sbrk fails in allocating the needed space, return NULL. 
    */
   payload_start return_pointer = smalloc_helper_find_avalible(size);

   if (return_pointer != nullptr){
    return return_pointer;
   }
}
payload_start smalloc_helper_find_avalible(size_t size){}
actual_block_start actually_allocate(size_t size){}

void* scalloc(size_t num, size_t size){
    /*
    ● Searches for a free block of at least ‘num’ elements, each ‘size’ bytes that are all set to 0
    or allocates if none are found. In other words, find/allocate size * num bytes and set all
    bytes to 0.
    ● Return value:
    i. Success - returns pointer to the first byte in the allocated block.
    ii. Failure –
    a. If size or num is 0 returns NULL.
    b. If ‘size * num’ is more than 108, return NULL.
    c. If sbrk fails in allocating the needed space, return NULL.
    */
}

void sfree(void* p){
    /*
    ● Releases the usage of the block that starts with the pointer ‘p’.
    ● If ‘p’ is NULL or already released, simply returns.
    ● Presume that all pointers ‘p’ truly points to the beginning of an allocated block.
    */
}

void* srealloc(void* oldp, size_t size){
    /*
    ● If ‘size’ is smaller than or equal to the current block’s size, reuses the same block.
Otherwise, finds/allocates ‘size’ bytes for a new space, copies content of oldp into the
new allocated space and frees the oldp.
● Return value:
i. Success –
a. Returns pointer to the first byte in the (newly) allocated space.
b. If ‘oldp’ is NULL, allocates space for ‘size’ bytes and returns a pointer to it.
ii. Failure –
a. If size is 0 returns NULL.
b. If ‘size’ if more than 108, return NULL.
c. If sbrk fails in allocating the needed space, return NULL.
d. Do not free ‘oldp’ if srealloc() fails. 
    */
}

size_t _num_free_blocks(){
    /*
    ● Returns the number of allocated blocks in the heap that are currently free. 
    */
}
size_t _num_free_bytes(){
    /*
    ● Returns the number of bytes in all allocated blocks in the heap that are currently free,
excluding the bytes used by the meta-data structs. 
    */
}

size_t _num_allocated_blocks(){
    /*
    ● Returns the overall (free and used) number of allocated blocks in the heap. 
    */
}

size_t _num_allocated_bytes(){
    /*
    ● Returns the overall number (free and used) of allocated bytes in the heap, excluding
the bytes used by the meta-data structs. 
    */
}

size_t _num_meta_data_bytes(){
    /*
    ● Returns the overall number of meta-data bytes currently in the heap. 
    */
}

size_t _size_meta_data(){
    /*
    ● Returns the number of bytes of a single meta-data structure in your system.
    */
}


enum MemoryStatus {
    FREE = 0,
    ALLOCATED = 1
};

typedef unsigned long size_t; //FIXME: delete this line! its only for my mac

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