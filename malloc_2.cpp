typedef unsigned long size_t;    // FIXME: delete this line! its only for my mac
void *sbrk(payload_size_t payload_size); // FIXME: delete this line! its only for my mac

#include <string.h>

#define ESER_BECHEZKAT_SHMONE (100000000)
#define BLOCK_BUFFER_SIZE ((sizeof(MallocMetadata)))
#define SYSCALL_FAILED(POINTER) (((long int)POINTER) == -1)

/* defines for things we need to ask for in the piazza */
#define ACCOUNT_FOR__size_meta_meta_data (1)  // <- if we do not need to account for size of head_dummy, tail_dummy etc then flip this flag to 0
#define IS_OK_TO_INCLUDE_ASSERT (1)           // <- if we can not include assert, flip flag to 0.
#define HARD_TYPE_CHECK (1)                   // <- controls whether our custom types are enforced by the constructor (=1) or not (=0).

#if IS_OK_TO_INCLUDE_ASSERT
#include <cassert>
#else //IS_OK_TO_INCLUDE_ASSERT
#define assert(expr) ((void)0) // <- if we can not include assert, this is (apperantly) a valid no-op statement.
#endif //IS_OK_TO_INCLUDE_ASSERT

#if HARD_TYPE_CHECK
struct payload_start {
    void* ptr;

    // Implicit conversion FROM void*
    payload_start(void* p) : ptr(p) {}

    // Implicit conversion TO void*
    operator void*() const { return ptr; }
};

struct actual_block_start {
    void* ptr;

    // Implicit conversion FROM void*
    actual_block_start(void* p) : ptr(p) {}

    // Implicit conversion TO void*
    operator void*() const { return ptr; }
};

struct payload_size_t {
    size_t value;

    // Implicit conversion from size_t
    payload_size_t(size_t v) : value(v) {}

    // Implicit conversion to size_t
    operator size_t() const { return value; }
};

struct actual_size_t {
    size_t value;

    // Implicit conversion from size_t
    actual_size_t(size_t v) : value(v) {}

    // Implicit conversion to size_t
    operator size_t() const { return value; }
};
#else // HARD_TYPE_CHECK
/* typedef for clarity */
typedef void *payload_start;
typedef void *actual_block_start;
typedef size_t payload_size_t;
typedef size_t actual_size_t;
#endif // HARD_TYPE_CHECK

/* foward declarations */
struct MallocMetadata;

/* functions from the hw */
payload_start smalloc(payload_size_t payload_size);
payload_start scalloc(size_t num, size_t payload_size);
void sfree(payload_start p);
payload_start srealloc(void *oldp, payload_size_t payload_size);
size_t _num_free_blocks();
payload_size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes(); 
size_t _num_meta_data_bytes();
size_t _size_meta_data();

/* our helper functions */
payload_start smalloc_helper_find_avalible(payload_size_t payload_size);
actual_block_start actually_allocate(actual_size_t actual_block_size);
inline void markFree(payload_start block);
inline void markAllocated(payload_start block);
inline payload_size_t getBlockSize(payload_start block);
inline bool isAllocated(payload_start block);
inline bool isFree(payload_start block);
inline bool isSizeValid(payload_size_t payload_size);
inline void initializeList();
inline payload_start _initBlock_MetaData(actual_block_start block, actual_size_t actual_block_size);
inline payload_start initAllocatedBlock(actual_block_start block, actual_size_t actual_block_size);
inline payload_start initFreeBlock(actual_block_start block, actual_size_t actual_block_size);
inline MallocMetadata *getMallocStruct(payload_start block);
inline size_t _size_meta_meta_data();
inline MallocMetadata *getGlobalMallocStructHeadFree(); 
inline MallocMetadata *getNextMallocBlock(MallocMetadata *current_block);
inline MallocMetadata *getGlobalMallocStructTailFree(); 
inline payload_start getStructsPayload(MallocMetadata *malloc_of_block);
inline void _placeBlockInFreeList(MallocMetadata *malloc_manager_of_block);
inline MallocMetadata* _firstBlockAfter(MallocMetadata *malloc_manager_of_block);

/* memory management meta data struct */
struct MallocMetadata
{
    payload_size_t payload_size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
};

static MallocMetadata head_dummy = {0, true, nullptr, nullptr};
static MallocMetadata tail_dummy = {0, true, nullptr, nullptr};
static bool is_list_initialized = false;
static size_t num_allocated_blocks = 0;
static payload_size_t num_allocated_bytes = 0;




payload_start smalloc(payload_size_t payload_size)
{
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
    if (!isSizeValid(payload_size))
    {
        return nullptr;
    }
    initializeList();
    payload_start look_for_avalible = smalloc_helper_find_avalible(payload_size);

    if (look_for_avalible != nullptr)
    {
        markAllocated(look_for_avalible);
        return look_for_avalible;
    }

    actual_size_t temp_size = (actual_size_t) payload_size + BLOCK_BUFFER_SIZE;
    actual_block_start temp = actually_allocate(temp_size);
    if (temp == nullptr)
    { // allocation might have failed
        return nullptr;
    }
    payload_start new_allocation = initAllocatedBlock(temp, temp_size);
    num_allocated_blocks++;
    num_allocated_bytes = (((size_t)num_allocated_bytes) + ((size_t)payload_size)); // num_allocated_bytes += payload_size;
    return new_allocation;
}

payload_start scalloc(size_t num, size_t size)
{
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
    payload_size_t actual_payload_size = num * size;
    payload_start allocated_block = smalloc(actual_payload_size);
    if (allocated_block == nullptr)
    {
        return nullptr;
    }

    memset(allocated_block, 0, actual_payload_size);
    return allocated_block;
}

void sfree(void *p)
{
    /*
    ● Releases the usage of the block that starts with the pointer ‘p’.
    ● If ‘p’ is NULL or already released, simply returns.
    ● Presume that all pointers ‘p’ truly points to the beginning of an allocated block.
    */
    markFree(p); // <- markFree also handles metadata operations.
}

payload_start srealloc(payload_start oldp, payload_size_t payload_size)
{
    /*
    ● If 'payload_size' is smaller than or equal to the current block’s size, reuses the same block.
      Otherwise, finds/allocates 'payload_size' bytes for a new space, copies content of oldp into the
      new allocated space and frees the oldp.
    ● Return value:
        i. Success –
        a. Returns pointer to the first byte in the (newly) allocated space.
        b. If ‘oldp’ is NULL, allocates space for 'payload_size' bytes and returns a pointer to it.
        ii. Failure –
        a. If size is 0 returns NULL.
        b. If 'payload_size' if more than 10**8, return NULL.
        c. If sbrk fails in allocating the needed space, return NULL.
        d. Do not free ‘oldp’ if srealloc() fails.
    */
    if (!isSizeValid(payload_size))
    {
        return nullptr;
    }
    if (oldp == nullptr)
    {
        payload_start temp = smalloc(payload_size);
        if (temp != nullptr)
        {
            num_allocated_blocks++;
            num_allocated_bytes = (((size_t)num_allocated_bytes) + ((size_t)payload_size)); // num_allocated_bytes += payload_size;
        }
        return temp;
    }
    MallocMetadata *oldp_metadata = getMallocStruct(oldp);
    payload_size_t oldp_size = oldp_metadata->payload_size;
    if (oldp_size >= payload_size)
    {
        return oldp;
    }
    payload_start new_block = smalloc(payload_size);
    if (new_block == nullptr)
    {
        return nullptr;
    }
    memmove(new_block, oldp, oldp_size);
    sfree(oldp);
    return new_block;
}

size_t _num_free_blocks()
{
    /*
    ● Returns the number of allocated blocks in the heap that are currently free.
    */

    size_t free_blocks = 0;

    MallocMetadata *global_head = getGlobalMallocStructHeadFree();
    MallocMetadata *global_tail = getGlobalMallocStructTailFree();
    for (MallocMetadata *temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
    {
        ++free_blocks;
    }

    return free_blocks;
}

payload_size_t _num_free_bytes()
{
    /*
    ● Returns the number of bytes in all allocated blocks in the heap that are currently free,
      excluding the bytes used by the meta-data structs.
    */

    payload_size_t free_bytes = 0;

    MallocMetadata *global_head = getGlobalMallocStructHeadFree();
    MallocMetadata *global_tail = getGlobalMallocStructTailFree();
    for (MallocMetadata *temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
    {
        free_bytes += temp->payload_size;
    }

    return free_bytes;
}

size_t _num_allocated_blocks()
{
    /*
    ● Returns the overall (free and used) number of allocated blocks in the heap.
    */
    return num_allocated_blocks;
}

size_t _num_allocated_bytes()
{
    /*
    ● Returns the overall number (free and used) of allocated bytes in the heap, excluding
      the bytes used by the meta-data structs.
    */
    return num_allocated_bytes;
}

size_t _num_meta_data_bytes()
{
    /*
    ● Returns the overall number of meta-data bytes currently in the heap.
    */
    return _size_meta_data() * _num_allocated_blocks() + _size_meta_meta_data();
}

size_t _size_meta_data()
{
    /*
    ● Returns the number of bytes of a single meta-data structure in your system.
    */
    return sizeof(MallocMetadata);
}

inline size_t _size_meta_meta_data()
{   // <- TODO: ask in the piazza if this is needed, if not simply return 0
    /*
    ● Returns the number of bytes of a meta-data in your system that are not related to the blocks.
    */
    #if ACCOUNT_FOR__size_meta_meta_data
    size_t bytes_for_linked_list_head_and_tail = 2*sizeof(MallocMetadata);
    size_t bytes_for_initialised_linked_list_bool = sizeof(bool);
    size_t bytes_for_allocation_counters = 2*sizeof(size_t);
    size_t total_meta_metadata_bytes = (bytes_for_linked_list_head_and_tail + bytes_for_initialised_linked_list_bool + bytes_for_allocation_counters);
    return total_meta_metadata_bytes;
    #else // ACCOUNT_FOR__size_meta_meta_data
    return 0;
    #endif // ACCOUNT_FOR__size_meta_meta_data
}

payload_start smalloc_helper_find_avalible(payload_size_t payload_size)
{
    // finds first avalible block with capacity that is at least what is requested (no fancy math)
    // only return the pointer to the payload, not the struct itself.
    MallocMetadata *global_head = getGlobalMallocStructHeadFree();
    MallocMetadata *global_tail = getGlobalMallocStructTailFree();
    for (MallocMetadata *temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
    {
        if (temp->payload_size >= payload_size)
        {
            payload_start fitting_block = getStructsPayload(temp);
            return fitting_block;
        }
    }
    return nullptr;
}

actual_block_start actually_allocate(actual_size_t actual_block_size)
{ // literally copy paste of the previous part.
    if (actual_block_size <= 0 || actual_block_size > ESER_BECHEZKAT_SHMONE)
    {
        return nullptr;
    }

    void *meta_block_start = sbrk(0);

    if (SYSCALL_FAILED(meta_block_start))
    {
        return nullptr;
    }

    void *one_after_meta_block_end = sbrk(actual_block_size);

    if (SYSCALL_FAILED(one_after_meta_block_end))
    {
        return nullptr;
    }

    return (actual_block_start) meta_block_start;
}

inline bool isAllocated(payload_start block)
{
    return !(isFree(block));
}

inline bool isFree(payload_start block)
{
    MallocMetadata *blocks_metadata_manager = getMallocStruct(block);
    return blocks_metadata_manager->is_free;
}

inline bool isSizeValid(payload_size_t payload_size)
{
    return ((payload_size > 0) && (payload_size <= ESER_BECHEZKAT_SHMONE));
}

inline void initializeList()
{
    if (!is_list_initialized){
     head_dummy.next = &tail_dummy;
     tail_dummy.prev = &head_dummy;
     is_list_initialized = true;
    }
}

inline void markAllocated(payload_start block)
{
    // mark bool as allocated, remove from doubly linked list,
    //  IMPORTANT: regards pointers in doubly linked list as valid data (no null and no garbage)
    MallocMetadata *blocks_metadata_manager = getMallocStruct(block);

    assert(blocks_metadata_manager->next != nullptr); // <- cannot verify garbage though.
    assert(blocks_metadata_manager->prev != nullptr);
    assert(blocks_metadata_manager->is_free); // <- might be redundant

    // update flag
    blocks_metadata_manager->is_free = false;

    // remove from doubly linked list
    blocks_metadata_manager->prev->next = blocks_metadata_manager->next;
    blocks_metadata_manager->next->prev = blocks_metadata_manager->prev;

    // truncate pointers to prevent undefined/ unexpected behaviour
    blocks_metadata_manager->next = nullptr;
    blocks_metadata_manager->prev = nullptr;
}

inline void markFree(payload_start block)
{
    // mark bool as free, add to doubly linked list, regard previous pointers in the block meta data as garbage.
    //  IMPORTANT: NO DOUBLE FREE CALLS!
    MallocMetadata *blocks_metadata_manager = getMallocStruct(block);

    assert(isAllocated(block)); // <- NO DOUBLE FREE! thats why _initBlock_MetaData needs to initialise MallocMetadata->is_free = false.

    // mark free
    blocks_metadata_manager->is_free = true;

    _placeBlockInFreeList(blocks_metadata_manager); // <- expects a free marked block!
}

inline payload_size_t getBlockSize(payload_start block)
{
    MallocMetadata *blocks_metadata_manager = getMallocStruct(block);
    return blocks_metadata_manager->payload_size;
}

inline payload_start _initBlock_MetaData(actual_block_start block, actual_size_t actual_block_size)
{
    // create the metadata with regards to the actuall block start, and return the payload block start.
    // need to initialise: MallocMetadata struct, MallocMetadata->payload_size = actual_block_size, MallocMetadata->is_free = false, other MallocMetadata fields can be garbage.
    // IMPORTANT TO FOLLOW THESE INITIALISATIONS, other functions do not check for validity of data for speed,
    // therefore these fields must be initialised for these values!

    MallocMetadata *meta_data = (MallocMetadata *)block;
    meta_data->payload_size = actual_block_size - BLOCK_BUFFER_SIZE;
    meta_data->is_free = false;
    meta_data->next = nullptr;
    meta_data->prev = getGlobalMallocStructTailFree();

    return getStructsPayload(meta_data);
}

inline payload_start initAllocatedBlock(actual_block_start block, actual_size_t actual_block_size)
{
    payload_start temp = initFreeBlock(block, actual_block_size); // <- initFreeBlock initialises the linked list
    markAllocated(temp);                                          // <- mark allocated removes from linked list, make sure linked list is initialised.
    return temp;
}

inline payload_start initFreeBlock(actual_block_start block, actual_size_t actual_block_size)
{
    payload_start temp = _initBlock_MetaData(block, actual_block_size);
    markFree(temp); // <- put block in the free linked list, and update markers.
    return temp;
}

inline MallocMetadata *getMallocStruct(payload_start block)
{
    MallocMetadata *temp = (MallocMetadata *)block;
    MallocMetadata *meta_data = temp - 1;
    return meta_data;
}

inline MallocMetadata *getGlobalMallocStructHeadFree()
{
    return &head_dummy;
}

inline MallocMetadata *getNextMallocBlock(MallocMetadata *current_block)
{
    // if (current_block == nullptr){ return nullptr;}
    assert(current_block != nullptr);
    return current_block->next;
}

inline MallocMetadata *getGlobalMallocStructTailFree()
{
    return &tail_dummy;
}

inline payload_start getStructsPayload(MallocMetadata *malloc_of_block)
{
    MallocMetadata *temp = malloc_of_block + 1;
    payload_start tmp_payload_start = (payload_start)temp;
    return tmp_payload_start;
}

inline void _placeBlockInFreeList(MallocMetadata *malloc_manager_of_block){
    assert((malloc_manager_of_block!=nullptr) && "in function '_placeBlockInFreeList': recieved nullptr as \"(MallocMetadata *malloc_manager_of_block\" argument.");

    // add to doubly linked list, regard previous pointers in the block meta data as garbage.
    MallocMetadata *global_head = getGlobalMallocStructHeadFree();
    MallocMetadata *blocks_metadata_manager = getMallocStruct(malloc_manager_of_block);

    // make sure block was marked as free
    assert(malloc_manager_of_block->is_free && "in function '_placeBlockInFreeList': block was not marked as free before insertion attempt into the free linked list.");

    // find the block that would be after the current block in the free linked list
    MallocMetadata* firstBlockAfter = _firstBlockAfter(malloc_manager_of_block);
    assert((firstBlockAfter!=nullptr) && "in function '_placeBlockInFreeList': block after was returned as nullptr.");

    // adjust new blocks pointers
    malloc_manager_of_block->next = firstBlockAfter;
    malloc_manager_of_block->prev = firstBlockAfter->prev;

    // truncate (update) old pointers
    firstBlockAfter->prev = malloc_manager_of_block;
    malloc_manager_of_block->prev->next = malloc_manager_of_block;
}

inline MallocMetadata* _firstBlockAfter(MallocMetadata *malloc_manager_of_block){
    assert((malloc_manager_of_block!=nullptr) && "in function '_firstBlockAfter': recieved nullptr as \"(MallocMetadata *malloc_manager_of_block\" argument.");

    MallocMetadata *global_head = getGlobalMallocStructHeadFree();
    MallocMetadata *global_tail = getGlobalMallocStructTailFree();
    MallocMetadata *temp;

    // find the first block (after head and before tail) with a higher addres than current adress, if there isnt then return tail.
    for (temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
    {
        if (malloc_manager_of_block < temp){break;} // we are comparing the addreses themselves! not the value of the pointers!
    }

    assert((temp!=nullptr) && "in function '_firstBlockAfter': temp was resolved to nullptr, see code for comment on what to do.");
    // if the assert failed: need to add a break condition in the for loop or a condition after the loop to make a nullptr return tail.

    return temp;
}

