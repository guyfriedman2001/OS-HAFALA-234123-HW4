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
inline void markAllocated(payload_start block);
inline size_t getBlockSize(payload_start block);
inline bool isAllocated(payload_start block);
inline bool isFree(payload_start block);
inline payload_start _initBlock_MetaData(actual_block_start block, size_t actual_block_size);
inline payload_start initAllocatedBlock(actual_block_start block, size_t actual_block_size);
inline payload_start initFreeBlock(actual_block_start block, size_t actual_block_size);

/* memory management meta data struct */
struct MallocMetadata {
 size_t size;
 bool is_free;
MallocMetadata* next;
 MallocMetadata* prev;
};

#define ESER_BECHEZKAT_SHMONE (100000000)
#define BLOCK_BUFFER_SIZE ((sizeof(MallocMetadata)))
#define SYSCALL_FAILED(POINTER) (((long int) POINTER) == -1)



payload_start smalloc(size_t size){
    /*
    ● Searches for a free block with at least ‘size’ bytes or allocates (sbrk()) one if none are
found.
● Return value:
i. Success – returns pointer to the first byte in the allocated block (excluding the meta-data of
course)
ii. Failure –
a. If size is 0 returns NULL.
b. If ‘size’ is more than 10**8, return NULL.
c. If sbrk fails in allocating the needed space, return NULL. 
    */
   if (size <= 0 || size > ESER_BECHEZKAT_SHMONE){
    return nullptr;
   }

   payload_start look_for_avalible = smalloc_helper_find_avalible(size);

   if (look_for_avalible != nullptr){
    markAllocated(look_for_avalible);
    return look_for_avalible;
   }

   size_t temp_size = size+BLOCK_BUFFER_SIZE;
   actual_block_start temp = actually_allocate(temp_size);
   payload_start new_allocation = initAllocatedBlock(temp,temp_size);
   return new_allocation;

}

payload_start smalloc_helper_find_avalible(size_t size){}

actual_block_start actually_allocate(size_t size){ //literally copy paste of the previous part.
        if (size <= 0 || size > 100000000){
        return nullptr;
    }

    void* meta_block_start = sbrk(0);

    if (SYSCALL_FAILED(meta_block_start)){
        return nullptr;
    }

    void* one_after_meta_block_end = sbrk(size);

    if (SYSCALL_FAILED(one_after_meta_block_end)){
        return nullptr;
    }

    return meta_block_start;
}

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




inline bool isAllocated(payload_start block){
    //TODO: 
}

inline bool isFree(payload_start block){
    return !(isAllocated(block));
}



inline void markAllocated(payload_start block){
    //TODO: 
}

inline void markFree(payload_start block){
    //TODO: 
}

inline size_t getBlockSize(payload_start block){
    //TODO: 
}

inline payload_start _initBlock_MetaData(actual_block_start block, size_t actual_block_size){
    //TODO: 
}

inline payload_start initAllocatedBlock(actual_block_start block, size_t actual_block_size){
    payload_start temp = _initBlock_MetaData(block, actual_block_size);
    markAllocated(temp);
    return temp;
}

inline payload_start initFreeBlock(actual_block_start block, size_t actual_block_size){
    //TODO: 
}