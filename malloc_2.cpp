typedef unsigned long size_t; //FIXME: delete this line! its only for my mac
void* sbrk(size_t size); //FIXME: delete this line! its only for my mac

#include <cassert>

/* typedef for clarity */
typedef void* payload_start;
typedef void* actual_block_start;

/* foward declarations */
struct MallocMetadata;

/* functions from the hw */
payload_start smalloc(size_t size);
payload_start scalloc(size_t num, size_t size);
void sfree(payload_start p);
payload_start srealloc(void* oldp, size_t size); // <- TODO:
size_t _num_free_blocks(); // <- TODO:
size_t _num_free_bytes(); // <- TODO:
size_t _num_allocated_blocks(); // <- TODO:
size_t _num_allocated_bytes(); // <- TODO:
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
inline payload_start _initBlock_MetaData(actual_block_start block, size_t actual_block_size); // <- TODO:
inline payload_start initAllocatedBlock(actual_block_start block, size_t actual_block_size);
inline payload_start initFreeBlock(actual_block_start block, size_t actual_block_size);
inline MallocMetadata* getMallocStruct(payload_start block); // <- TODO:
size_t _size_meta_meta_data(); // <- TODO:
inline MallocMetadata* getGlobalMallocStructHead(); // <- TODO:
inline MallocMetadata* getNextMallocBlock(MallocMetadata* current_block);
inline MallocMetadata* getGlobalMallocStructTail(); // <- TODO:
inline payload_start getStructsPayload(MallocMetadata* malloc_of_block); // <- TODO:



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

void* scalloc(size_t num, size_t size){
    /*
    ● Searches for a free block of at least ‘num’ elements, each ‘size’ bytes that are all set to 0
    or allocates if none are found. In other words, find/allocate size * num bytes and set all
    bytes to 0.
    ● Return value:
    i. Success - returns pointer to the first byte in the allocated block.
    ii. Failure –
    a. If size or num is 0 returns NULL.
    b. If ‘size * num’ is more than 10**8, return NULL.
    c. If sbrk fails in allocating the needed space, return NULL.
    */
   size_t actual_size = num*size;
   payload_start allocated_block = smalloc(actual_size);
   if (allocated_block == nullptr){
    return nullptr;
   }
   //FIXME: USE MMSET!!!
   for (size_t i = 0; i < actual_size; ++i){
    ((char*)allocated_block)[i] = 0;
   }
   return allocated_block;
}

void sfree(void* p){
    /*
    ● Releases the usage of the block that starts with the pointer ‘p’.
    ● If ‘p’ is NULL or already released, simply returns.
    ● Presume that all pointers ‘p’ truly points to the beginning of an allocated block.
    */
   markFree(p); // <- markFree also handles metadata operations.
}

payload_start srealloc(payload_start oldp, size_t size){
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
        b. If ‘size’ if more than 10**8, return NULL.
        c. If sbrk fails in allocating the needed space, return NULL.
        d. Do not free ‘oldp’ if srealloc() fails. 
    */
   //TODO:
}

size_t _num_free_blocks(){
    /*
    ● Returns the number of allocated blocks in the heap that are currently free. 
    */
   //TODO:
}

size_t _num_free_bytes(){
    /*
    ● Returns the number of bytes in all allocated blocks in the heap that are currently free,
excluding the bytes used by the meta-data structs. 
    */
   //TODO:
}

size_t _num_allocated_blocks(){
    /*
    ● Returns the overall (free and used) number of allocated blocks in the heap. 
    */
   //TODO:
}

size_t _num_allocated_bytes(){
    /*
    ● Returns the overall number (free and used) of allocated bytes in the heap, excluding
      the bytes used by the meta-data structs. 
    */
   //TODO:
}

size_t _num_meta_data_bytes(){
    /*
    ● Returns the overall number of meta-data bytes currently in the heap. 
    */
    return _size_meta_data()*_num_allocated_blocks() + _size_meta_meta_data();
}

size_t _size_meta_data(){
    /*
    ● Returns the number of bytes of a single meta-data structure in your system.
    */
   return sizeof(MallocMetadata);
}

size_t _size_meta_meta_data(){ // <- TODO: ask in the piazza if this is needed
    /*
    ● Returns the number of bytes of a meta-data in your system that are not related to the blocks.
    */
   return -1;
}


payload_start smalloc_helper_find_avalible(size_t size){
    //finds first avalible block with capacity that is at least what is requested (no fancy math)
    //only return the pointer to the payload, not the struct itself.
    MallocMetadata* global_head = getGlobalMallocStructHead();
    MallocMetadata* global_tail = getGlobalMallocStructTail();
    for (MallocMetadata* temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp)){
        if(temp->size >= size){
            return getStructsPayload(temp);
        }
    }
    return nullptr;
}

actual_block_start actually_allocate(size_t size){ //literally copy paste of the previous part.
        if (size <= 0 || size > ESER_BECHEZKAT_SHMONE){
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



inline bool isAllocated(payload_start block){
    return !(isFree(block));
}

inline bool isFree(payload_start block){
    MallocMetadata* blocks_metadata_manager = getMallocStruct(block);
    return blocks_metadata_manager->is_free;
}



inline void markAllocated(payload_start block){
    //mark bool as allocated, remove from doubly linked list, 
    // IMPORTANT: regards pointers in doubly linked list as valid data (no null and no garbage)
    MallocMetadata* blocks_metadata_manager = getMallocStruct(block);

    assert(blocks_metadata_manager->next != nullptr); // <- cannot verify garbage though.
    assert(blocks_metadata_manager->prev != nullptr);
    assert(blocks_metadata_manager->is_free); // <- might be redundant

    //update flag
    blocks_metadata_manager->is_free = false;

    //remove from doubly linked list
    blocks_metadata_manager->prev->next = blocks_metadata_manager->next;
    blocks_metadata_manager->next->prev = blocks_metadata_manager->prev;

    //truncate pointers to prevent undefined/ unexpected behaviour
    blocks_metadata_manager->next = nullptr;
    blocks_metadata_manager->prev = nullptr;
}

inline void markFree(payload_start block){
    //mark bool as free, add to doubly linked list, regard previous pointers in the block meta data as garbage.
    // IMPORTANT: NO DOUBLE FREE CALLS!
    MallocMetadata* global_head = getGlobalMallocStructHead();
    MallocMetadata* blocks_metadata_manager = getMallocStruct(block);

    assert(isAllocated(block)); // <- NO DOUBLE FREE! thats why _initBlock_MetaData needs to initialise MallocMetadata->is_free = false.

    //adjust new blocks pointers
    blocks_metadata_manager->next = global_head->next;
    blocks_metadata_manager->prev = global_head;

    //truncate old pointers
    global_head->next->prev = blocks_metadata_manager;
    global_head->next = blocks_metadata_manager;

    //mark free
    blocks_metadata_manager->is_free = true;
}

inline size_t getBlockSize(payload_start block){
    MallocMetadata* blocks_metadata_manager = getMallocStruct(block);
    return blocks_metadata_manager->size;
}

inline payload_start _initBlock_MetaData(actual_block_start block, size_t actual_block_size){
    //TODO: create the metadata with regards to the actuall block start, and return the payload block start.
    // need to initialise: MallocMetadata struct, MallocMetadata->size = actual_block_size, MallocMetadata->is_free = false, other MallocMetadata fields can be garbage.
    // IMPORTANT TO FOLLOW THESE INITIALISATIONS, other functions do not check for validity of data for speed,
    // therefore these fields must be initialised for these values!
}

inline payload_start initAllocatedBlock(actual_block_start block, size_t actual_block_size){
    payload_start temp = initFreeBlock(block, actual_block_size); // <- initFreeBlock initialises the linked list
    markAllocated(temp); // <- mark allocated removes from linked list, make sure linked list is initialised.
    return temp;
}

inline payload_start initFreeBlock(actual_block_start block, size_t actual_block_size){
    payload_start temp = _initBlock_MetaData(block, actual_block_size);
    markFree(temp); // <- put block in the free linked list, and update markers.
    return temp;
}

inline MallocMetadata* getMallocStruct(payload_start block){
    //TODO:
}

inline MallocMetadata* getGlobalMallocStructHead(){
    //TODO: return the head deme (יעני ראש דמה) of our global doubly linked list of free blocks
}

inline MallocMetadata* getNextMallocBlock(MallocMetadata* current_block){
    //if (current_block == nullptr){ return nullptr;}
    assert(current_block != nullptr);
    return current_block->next;
}

inline MallocMetadata* getGlobalMallocStructTail(){
    //TODO: return the tail deme (יעני זנב דמה) of our global doubly linked list of free blocks
}

inline payload_start getStructsPayload(MallocMetadata* malloc_of_block){
    //TODO: return payload pointer of the block managed by the struct
}