// typedef unsigned long size_t;    // FIXME: delete this line! its only for my mac
// void *sbrk(payload_size_t payload_size); // FIXME: delete this line! its only for my mac

#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/mman.h>


#define ESER_BECHEZKAT_SHMONE (100'000'000) //((size_t)1e8) // (100000000)
#define BLOCK_BUFFER_SIZE ((sizeof(MallocMetadata)))
#define SYSCALL_FAILED(POINTER) (((long int)POINTER) == -1)

/* defines for things we need to ask for in the piazza */
#define ACCOUNT_FOR__size_meta_meta_data (0) // <- if we do not need to account for size of head_dummy, tail_dummy etc then flip this flag to 0
#define IS_OK_TO_INCLUDE_ASSERT (1)          // <- if we can not include assert, flip flag to 0.
#define HARD_TYPE_CHECK (0)                  // <- controls whether our custom types are enforced by the compiler (=1) or not (=0).
#define NUM_ORDERS ((size_t)11)              // <- number of 'orders' our code will handle, as per the instructions.

#if IS_OK_TO_INCLUDE_ASSERT
#include <cassert>
#else                          // IS_OK_TO_INCLUDE_ASSERT
#define assert(expr) ((void)0) // <- if we can not include assert, this is (apperantly) a valid no-op statement.
#endif                         // IS_OK_TO_INCLUDE_ASSERT

#if HARD_TYPE_CHECK
struct payload_start
{
    void *ptr;

    // Implicit conversion FROM void*
    payload_start(void *p) : ptr(p) {}

    // Implicit conversion TO void*
    operator void *() const { return ptr; }
};

struct actual_block_start
{
    void *ptr;

    // Implicit conversion FROM void*
    actual_block_start(void *p) : ptr(p) {}

    // Implicit conversion TO void*
    operator void *() const { return ptr; }
};

struct payload_size_t
{
    size_t value;

    // Implicit conversion from size_t
    payload_size_t(size_t v) : value(v) {}

    // Implicit conversion to size_t
    operator size_t() const { return value; }
};

struct actual_size_t
{
    size_t value;

    // Implicit conversion from size_t
    actual_size_t(size_t v) : value(v) {}

    // Implicit conversion to size_t
    operator size_t() const { return value; }
};
#else  // HARD_TYPE_CHECK
/* typedef for clarity */
typedef void *payload_start;
typedef void *actual_block_start;
typedef size_t payload_size_t;
typedef size_t actual_size_t;
#endif // HARD_TYPE_CHECK

/* foward declarations */
struct MallocMetadata;

const size_t MAX_ORDER = NUM_ORDERS - 1;         
const size_t BLOCK_SIZE_BYTES = static_cast<size_t>(128) << MAX_ORDER;    
const size_t BLOCKS_IN_POOL = 32;                     
const size_t POOL_SIZE = BLOCK_SIZE_BYTES * BLOCKS_IN_POOL;

/* functions from the hw */
payload_start smalloc(payload_size_t payload_size);
payload_start scalloc(size_t num, size_t payload_size);
void sfree(payload_start p);
payload_start srealloc(payload_start oldp, payload_size_t payload_size);
size_t _num_free_blocks();
payload_size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

/* our helper functions */
payload_start smalloc_helper_find_avalible(payload_size_t payload_size);
actual_block_start smalloc_helper_break_existing(actual_size_t actual_block_size);
inline void markFree(payload_start block);
inline void markAllocated(payload_start block);
inline payload_size_t getBlockSize(payload_start block);
inline bool isAllocated(payload_start block);
inline bool isFree(payload_start block);
inline bool isSizeValid(payload_size_t payload_size);
inline void initializeList();
inline void initializeBuddy();
inline payload_start _initBlock_MetaData(actual_block_start block, actual_size_t actual_block_size, bool isMmap, size_t order);
inline payload_start initAllocatedBlock(actual_block_start block, actual_size_t actual_block_size, bool isMmap, size_t order);
inline payload_start initFreeBlock(actual_block_start block, actual_size_t actual_block_size, bool iMmap, size_t order);
inline MallocMetadata *getMallocStruct(payload_start block);
inline size_t _size_meta_meta_data();
inline MallocMetadata *getHeadAtOrder(size_t order);
inline MallocMetadata *getNextMallocBlock(MallocMetadata *current_block);
inline MallocMetadata *getTailAtOrder(size_t order);
inline payload_start getStructsPayload(MallocMetadata *malloc_of_block);
inline void _placeBlockInFreeList(MallocMetadata *malloc_manager_of_block);
inline MallocMetadata *_firstBlockAfter(MallocMetadata *malloc_manager_of_block);
inline void _init_dummy_MetaData(MallocMetadata* initialise_this);
inline size_t getOrderOfSize(payload_size_t size);
inline MallocMetadata *getHeadOfSize(payload_size_t payload_size);
inline MallocMetadata *getTailOfSize(payload_size_t payload_size);
inline void removeFromList(MallocMetadata* blk);
inline void insertToFreeList(MallocMetadata* blk);
inline MallocMetadata* splitBlock(MallocMetadata* blk, size_t target_order);
inline MallocMetadata* merge(MallocMetadata* blk);
inline void* getBuddyAddress(void* block, size_t order);
inline size_t orderToSize(size_t order);




/* memory management meta data struct */
struct MallocMetadata
{
    payload_size_t payload_size;
    bool is_free;
    bool is_mmap;
    size_t order;
    MallocMetadata *next;
    MallocMetadata *prev;
};

// new variables for part 3
static MallocMetadata head_dummy_array[NUM_ORDERS]; // <- all structs are not initialised and contain garbage
static MallocMetadata tail_dummy_array[NUM_ORDERS]; // <- all structs are not initialised and contain garbage
//static MallocMetadata head_dummy = {0, true, nullptr, nullptr};
//static MallocMetadata tail_dummy = {0, true, nullptr, nullptr};
static bool is_list_initialized = false;
static bool is_buddy_initialized = false;
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
    /*if (!isSizeValid(payload_size))
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

    actual_size_t temp_size = (actual_size_t)payload_size + BLOCK_BUFFER_SIZE;
    actual_block_start temp = smalloc_helper_break_existing(temp_size);
    if (temp == nullptr)
    { // allocation might have failed
        return nullptr;
    }
    payload_start new_allocation = initAllocatedBlock(temp, temp_size);
    num_allocated_blocks++;
    num_allocated_bytes = (((size_t)num_allocated_bytes) + ((size_t)payload_size)); // num_allocated_bytes += payload_size;
    return new_allocation;*/

    if (!isSizeValid(payload_size)) return nullptr;
    initializeList();                       

    if (payload_size + _size_meta_data() < BLOCK_SIZE_BYTES){
        if (!is_buddy_initialized)
            initializeBuddy();
        payload_start blk = smalloc_helper_find_avalible(payload_size);
        if (blk) return blk;   
    }
    actual_block_start region = smalloc_helper_break_existing(payload_size + _size_meta_data());
    if (!region) return nullptr;

    auto* meta = (MallocMetadata*)region;
    meta->payload_size = payload_size;
    meta->is_free = false;
    meta->is_mmap = true;
    meta->order = MAX_ORDER + 1;
    meta->next = meta->prev = nullptr;
    ++num_allocated_blocks;
    num_allocated_bytes += payload_size;
    return getStructsPayload(meta);                          
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
    /*payload_size_t actual_payload_size = num * size;
    payload_start allocated_block = smalloc(actual_payload_size);
    if (allocated_block == nullptr)
    {
        return nullptr;
    }

    memset(allocated_block, 0, actual_payload_size);
    return allocated_block;*/

    payload_size_t need = num * size;
    payload_start blk   = smalloc(need);
    if (!blk) return nullptr;

    memset(blk, 0, need);
    return blk;
}

void sfree(payload_start p)
{
    /*
    ● Releases the usage of the block that starts with the pointer ‘p’.
    ● If ‘p’ is NULL or already released, simply returns.
    ● Presume that all pointers ‘p’ truly points to the beginning of an allocated block.
    */
    //markFree(p); // <- markFree also handles metadata operations.

    if (!p) return;

    MallocMetadata* meta_data = getMallocStruct(p);

    if (meta_data->is_mmap)
    {
        munmap(static_cast<void*>(meta_data),
               static_cast<size_t>(meta_data->payload_size) + _size_meta_data());
        --num_allocated_blocks;
        num_allocated_bytes -= meta_data->payload_size;
        return;
    }
    if (meta_data->is_free)
        return;

    markFree(p);          
    merge(meta_data);         
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
    /*if (!isSizeValid(payload_size))
    {
        return nullptr;
    }
    if (oldp == nullptr)
    {
#if 0
        payload_start temp = smalloc(payload_size);
        if (temp != nullptr)
        {
            num_allocated_blocks++;
            num_allocated_bytes = (((size_t)num_allocated_bytes) + ((size_t)payload_size)); // num_allocated_bytes += payload_size;
        }
        return temp;
#else
        return smalloc(payload_size);
#endif
    }
    MallocMetadata *oldp_metadata = getMallocStruct(oldp);
    payload_size_t oldp_size = oldp_metadata->payload_size;
    if (((size_t)oldp_size) >= ((size_t)payload_size))
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
    return new_block;*/

    if (!isSizeValid(payload_size))
        return nullptr;

    if (!oldp)
        return smalloc(payload_size);

    MallocMetadata* meta = getMallocStruct(oldp);

    if (static_cast<size_t>(meta->payload_size) >= static_cast<size_t>(payload_size))
        return oldp;

    if (meta->is_mmap)
    {
        payload_start fresh = smalloc(payload_size);
        if (!fresh) return nullptr;
        memmove(fresh, oldp, meta->payload_size);
        sfree(oldp);
        return fresh;
    }

    payload_start fresh = smalloc(payload_size);
    if (!fresh) return nullptr;
    memmove(fresh, oldp, meta->payload_size);
    sfree(oldp);
    return fresh;
}

size_t _num_free_blocks()
{
    /*
    ● Returns the number of allocated blocks in the heap that are currently free.
    */
    initializeList();
    size_t free_blocks = 0;

    for (size_t j = 0; j < NUM_ORDERS; ++j)
    { // iterate over all possible orders.
        MallocMetadata *global_head = getHeadAtOrder(j);
        MallocMetadata *global_tail = getTailAtOrder(j);
        for (MallocMetadata *temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
        {
           if (temp->is_free) {
                ++free_blocks;
            }
        }
    }

    return free_blocks;
}

payload_size_t _num_free_bytes()
{
    /*
    ● Returns the number of bytes in all allocated blocks in the heap that are currently free,
      excluding the bytes used by the meta-data structs.
    */
    initializeList();
    payload_size_t free_bytes = 0;

    for (size_t j = 0; j < NUM_ORDERS; ++j)
    { // iterate over all possible orders.
        MallocMetadata *global_head = getHeadAtOrder(j);
        MallocMetadata *global_tail = getTailAtOrder(j);
        for (MallocMetadata *temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
        {
            free_bytes = (((size_t)free_bytes) + ((size_t)temp->payload_size)); // free_bytes += temp->payload_size;
        }
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
{ // <- TODO: ask in the piazza if this is needed, if not simply return 0
/*
● Returns the number of bytes of a meta-data in your system that are not related to the blocks.
*/
    #if ACCOUNT_FOR__size_meta_meta_data
    size_t bytes_for_linked_list_head_and_tail = 2*sizeof(MallocMetadata);
    size_t bytes_for_array = NUM_ORDERS*bytes_for_linked_list_head_and_tail;
    size_t bytes_for_initialised_linked_list_bool = sizeof(bool);
    size_t bytes_for_allocation_counters = 2*sizeof(size_t);
    size_t total_meta_metadata_bytes = (bytes_for_array + bytes_for_initialised_linked_list_bool + bytes_for_allocation_counters);
    return total_meta_metadata_bytes;
    #else // ACCOUNT_FOR__size_meta_meta_data
    return 0;
    #endif // ACCOUNT_FOR__size_meta_meta_data
}

payload_start smalloc_helper_find_avalible(payload_size_t payload_size)
{
    // finds first avalible block with capacity that is at least what is requested (no fancy math)
    // only return the pointer to the payload, not the struct itself.
    /*initializeList();
    MallocMetadata *global_head = getHeadOfSize(payload_size);
    MallocMetadata *global_tail = getTailOfSize(payload_size);
    MallocMetadata *temp;
    for (temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
    {
        if (((size_t)temp->payload_size) >= ((size_t)payload_size))
        {
            payload_start fitting_block = getStructsPayload(temp);
            return fitting_block;
        }
    }
    return nullptr;*/

    initializeList();
    size_t order_needed = getOrderOfSize(payload_size);

    for (size_t o = order_needed; o <= MAX_ORDER; ++o)
    {
        MallocMetadata* head = getHeadAtOrder(o);
        if (head->next == getTailAtOrder(o))         
            continue;

        MallocMetadata* blk = head->next;
        removeFromList(blk);                           
        blk = splitBlock(blk, order_needed);
        //++num_allocated_blocks; 
        //num_allocated_bytes += payload_size;          
        return getStructsPayload(blk);                 
    }
    return nullptr;   
}

actual_block_start smalloc_helper_break_existing(actual_size_t actual_block_size)
{ // literally copy paste of the previous part.
    /*if (((size_t)actual_block_size) <= 0 || ((size_t)actual_block_size) > ESER_BECHEZKAT_SHMONE)
    {
        return nullptr;
    }*/

    /*void *meta_block_start = sbrk(0);

    if (SYSCALL_FAILED(meta_block_start))
    {
        return nullptr;
    }

    void *one_after_meta_block_end = sbrk(actual_block_size);

    if (SYSCALL_FAILED(one_after_meta_block_end))
    {
        return nullptr;
    }

    return (actual_block_start)meta_block_start;*/

    void* region = mmap(nullptr, actual_block_size, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED)
        return nullptr;

    return static_cast<actual_block_start>(region);
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
    return ((((size_t)payload_size) > 0) && (((size_t)payload_size) <= ESER_BECHEZKAT_SHMONE));
}

inline void initializeList()
{
    if (is_list_initialized){return;}
    for (size_t i = 0; i < NUM_ORDERS; ++i){
        // create local variables for easy referencing
        MallocMetadata* head_dummy = &head_dummy_array[i];
        MallocMetadata* tail_dummy = &tail_dummy_array[i];

        // make sure no funny buisness is going on!
        assert((head_dummy!=nullptr) && "function 'initializeList' got 'head_dummy' == nullptr.");
        assert((tail_dummy!=nullptr) && "function 'initializeList' got 'tail_dummy' == nullptr.");

        // initial initialisation
        _init_dummy_MetaData(head_dummy);
        _init_dummy_MetaData(tail_dummy);

        // now make the pointers in the correct way
        head_dummy->next = tail_dummy;
        tail_dummy->prev = head_dummy;
    }

    // now after everything is initialised, update the flag.
    is_list_initialized = true;

    // now after the global variables are initialised, initialise all mem alloc (now it can be made in allignment)
    //initializeBuddy();
}

inline void _init_dummy_MetaData(MallocMetadata* initialise_this){

    // make sure no funny buisness is going on
    assert((initialise_this!=nullptr) && "function '_init_dummy_MetaData' got 'initialise_this' == nullptr.");

    //init to -> {0, true, nullptr, nullptr};
    initialise_this->payload_size = 0;
    initialise_this->is_free = true;
    initialise_this->is_mmap = false;
    initialise_this->order = 0;
    initialise_this->next = nullptr;
    initialise_this->prev = nullptr;
}

inline void initializeBuddy()
{
    assert(is_list_initialized && "function \"initializeBuddy\" was called before function \"initializeList\"! now allignment checking won't work!");
    if (is_buddy_initialized){return;}
    void* current_brk = sbrk(0);
    if (SYSCALL_FAILED(current_brk)) return;

    uintptr_t address   = (uintptr_t)current_brk;
    uintptr_t aligned = (address + POOL_SIZE - 1) & ~(POOL_SIZE - 1); // aligment
    if (aligned != address && SYSCALL_FAILED(sbrk(aligned - address)))
        return;

    void* region = sbrk(POOL_SIZE);
    if (SYSCALL_FAILED(region)) return;

    uint8_t* p = static_cast<uint8_t*>(region);
    for (size_t i = 0; i < BLOCKS_IN_POOL; ++i, p += BLOCK_SIZE_BYTES)
    {
        initFreeBlock((actual_block_start)p, BLOCK_SIZE_BYTES, false, MAX_ORDER);
    }
    is_buddy_initialized = true;
}

inline void markAllocated(payload_start block)
{
    // mark bool as allocated, remove from doubly linked list,
    //  IMPORTANT: regards pointers in doubly linked list as valid data (no null and no garbage)
    /*initializeList();
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
    blocks_metadata_manager->prev = nullptr;*/

    MallocMetadata* meta_data = getMallocStruct(block);
    if (!meta_data->is_free) return;                 
    meta_data->is_free = false;

    if (!meta_data->is_mmap) removeFromList(meta_data);
}

inline void markFree(payload_start block)
{
    
    /*initializeList();
    if (block == nullptr)
    {
        return;
    } // make it safe for freeing nullptr.
    // mark bool as free, add to doubly linked list, regard previous pointers in the block meta data as garbage.
    //  IMPORTANT: NO DOUBLE FREE CALLS!
    MallocMetadata *blocks_metadata_manager = getMallocStruct(block);

    assert(isAllocated(block)); // <- NO DOUBLE FREE! thats why _initBlock_MetaData needs to initialise MallocMetadata->is_free = false.

    // mark free
    blocks_metadata_manager->is_free = true;

    _placeBlockInFreeList(blocks_metadata_manager); // <- expects a free marked block!*/

    if (!block) return;
    MallocMetadata* meta_data = getMallocStruct(block);
    if (meta_data->is_mmap) {                      
        assert(false && "markFree called on mmap block");
        return;
    }
    meta_data->is_free = true;
    insertToFreeList(meta_data);
}

inline payload_size_t getBlockSize(payload_start block)
{
    MallocMetadata *blocks_metadata_manager = getMallocStruct(block);
    return blocks_metadata_manager->payload_size;
}

inline payload_start _initBlock_MetaData(actual_block_start block, actual_size_t actual_block_size, bool isMmap, size_t order)
{
    // create the metadata with regards to the actuall block start, and return the payload block start.
    // need to initialise: MallocMetadata struct, MallocMetadata->payload_size = actual_block_size, MallocMetadata->is_free = false, other MallocMetadata fields can be garbage.
    // IMPORTANT TO FOLLOW THESE INITIALISATIONS, other functions do not check for validity of data for speed,
    // therefore these fields must be initialised for these values!

    MallocMetadata *meta_data = (MallocMetadata *)(void *)block;
    meta_data->payload_size = (((size_t)actual_block_size) - BLOCK_BUFFER_SIZE);
    meta_data->is_mmap = isMmap;
    meta_data->order = order;
    meta_data->is_free = false;
    meta_data->next = nullptr;
    meta_data->prev = nullptr;
    return getStructsPayload(meta_data);
}

inline payload_start initAllocatedBlock(actual_block_start block, actual_size_t actual_block_size, bool isMmap, size_t order)
{
    payload_start temp = initFreeBlock(block, actual_block_size, isMmap, order); // <- initFreeBlock initialises the linked list
    if (!isMmap){
        markAllocated(temp);
    }                                          // <- mark allocated removes from linked list, make sure linked list is initialised.
    return temp;
}

inline payload_start initFreeBlock(actual_block_start block, actual_size_t actual_block_size, bool isMmap, size_t order)
{
    payload_start temp = _initBlock_MetaData(block, actual_block_size, isMmap, order);
    markFree(temp);
    ++num_allocated_blocks;
    num_allocated_bytes += ((size_t)actual_block_size - _size_meta_data());
    return temp;
}

inline MallocMetadata *getMallocStruct(payload_start block)
{
    MallocMetadata *temp = (MallocMetadata *)(void *)block;
    MallocMetadata *meta_data = temp - 1;
    return meta_data;
}

inline MallocMetadata *getHeadAtOrder(size_t order)
{
    return &head_dummy_array[order];
}

inline MallocMetadata *getNextMallocBlock(MallocMetadata *current_block)
{
    if (current_block == nullptr)
    {
        return nullptr;
    }
    // assert(current_block != nullptr);
    return current_block->next;
}

inline MallocMetadata *getTailAtOrder(size_t order)
{
    return &tail_dummy_array[order];
}

inline payload_start getStructsPayload(MallocMetadata *malloc_of_block)
{
    MallocMetadata *temp = malloc_of_block + 1;
    payload_start tmp_payload_start = (payload_start)temp;
    return tmp_payload_start;
}

inline void _placeBlockInFreeList(MallocMetadata *malloc_manager_of_block)
{
    /*assert((malloc_manager_of_block != nullptr) && "in function '_placeBlockInFreeList': recieved nullptr as \"(MallocMetadata *malloc_manager_of_block\" argument.");

    // add to doubly linked list, regard previous pointers in the block meta data as garbage.
    // MallocMetadata *global_head = getHeadAtOrder(int order);
    // MallocMetadata *blocks_metadata_manager = getMallocStruct(malloc_manager_of_block);

    // make sure block was marked as free
    assert(malloc_manager_of_block->is_free && "in function '_placeBlockInFreeList': block was not marked as free before insertion attempt into the free linked list.");

    // find the block that would be after the current block in the free linked list
    MallocMetadata *firstBlockAfter = _firstBlockAfter(malloc_manager_of_block);
    assert((firstBlockAfter != nullptr) && "in function '_placeBlockInFreeList': block after was returned as nullptr.");

    // adjust new blocks pointers
    malloc_manager_of_block->next = firstBlockAfter;
    malloc_manager_of_block->prev = firstBlockAfter->prev;

    // truncate (update) old pointers
    firstBlockAfter->prev = malloc_manager_of_block;
    malloc_manager_of_block->prev->next = malloc_manager_of_block;*/

    assert(malloc_manager_of_block && malloc_manager_of_block->is_free);
    size_t order = malloc_manager_of_block->order;

    MallocMetadata* head = getHeadAtOrder(order);
    MallocMetadata* tail = getTailAtOrder(order);

    MallocMetadata* curr = head->next;
    while (curr != tail && curr < malloc_manager_of_block) {
        curr = curr->next;
    }
    malloc_manager_of_block->next = curr;
    malloc_manager_of_block->prev = curr->prev;

    curr->prev->next = malloc_manager_of_block;
    curr->prev = malloc_manager_of_block;
}

inline MallocMetadata *_firstBlockAfter(MallocMetadata *malloc_manager_of_block)
{
    assert((malloc_manager_of_block != nullptr) && "in function '_firstBlockAfter': recieved nullptr as \"(MallocMetadata *malloc_manager_of_block\" argument.");

    size_t order = malloc_manager_of_block->order;
    MallocMetadata *head = getHeadAtOrder(order);
    MallocMetadata *tail = getTailAtOrder(order);
    //MallocMetadata *temp;

    // find the first block (after head and before tail) with a higher addres than current adress, if there isnt then return tail.
    /*for (temp = getNextMallocBlock(global_head); temp != global_tail; temp = getNextMallocBlock(temp))
    {
        if (malloc_manager_of_block <= temp)
        { // we are comparing the addreses themselves! not the value of the pointers!
            break;
        }
    }*/

    for (MallocMetadata* current = getNextMallocBlock(head);
         current != tail;
         current = getNextMallocBlock(current))
    {
        if (malloc_manager_of_block <= current)          // השוואה על-פי כתובת
            return current;
    }
    return tail;  

    /*assert((temp != nullptr) && "in function '_firstBlockAfter': temp was resolved to nullptr, see code for comment on what to do.");
    // if the assert failed: need to add a break condition in the for loop or a condition after the loop to make a nullptr return tail.

    return temp;*/
}

inline size_t getOrderOfSize(payload_size_t size){
    size_t needed = size + sizeof(MallocMetadata);   
    for (size_t order = 0; order <= MAX_ORDER; ++order) {
        if (needed <= orderToSize(order))
            return order;
    }
    return MAX_ORDER; 
}


inline MallocMetadata *getHeadOfSize(payload_size_t payload_size)
{
    return getHeadAtOrder(getOrderOfSize(payload_size));
}

inline MallocMetadata *getTailOfSize(payload_size_t payload_size)
{
    return getTailAtOrder(getOrderOfSize(payload_size));
}

inline void* getBuddyAddress(void* block, size_t order)
{
    return (void*)((uintptr_t)block ^ orderToSize(order));
}

inline void removeFromList(MallocMetadata* blk)
{
    assert(blk && blk->prev && blk->next &&
           "removeFromList: broken links!");
    blk->prev->next = blk->next;
    blk->next->prev = blk->prev;
    blk->next = blk->prev = nullptr;
}

inline void insertToFreeList(MallocMetadata* blk)
{
    assert(blk && blk->is_free);
    assert(blk->order <= MAX_ORDER);
    _placeBlockInFreeList(blk);     
}

inline MallocMetadata* splitBlock(MallocMetadata* big,
                                  size_t target_order)
{
    while (big->order > target_order)
    {
        const size_t new_order  = big->order - 1;
        const size_t half_bytes = orderToSize(new_order);

        uint8_t* big_start = (uint8_t*)big;      
        MallocMetadata* buddy = (MallocMetadata*)(big_start + half_bytes);

        big->order = new_order;
        big->payload_size = half_bytes - BLOCK_BUFFER_SIZE;
        buddy->order = new_order;
        buddy->payload_size = half_bytes - BLOCK_BUFFER_SIZE;
        buddy->is_free = true;
        buddy->is_mmap = false;
        buddy->next = buddy->prev = nullptr;
        insertToFreeList(buddy);
        ++num_allocated_blocks;
        num_allocated_bytes -= _size_meta_data();
    }

    big->is_free = false;
    return big;
}

inline MallocMetadata* merge(MallocMetadata* block)
{
    removeFromList(block);
    while (block->order < MAX_ORDER)
    {
        void* buddy_addr = getBuddyAddress(block, block->order);
        MallocMetadata* buddy = (MallocMetadata*)buddy_addr;
        if ((uintptr_t)buddy % orderToSize(block->order) != 0 || !buddy->is_free || buddy->order != block->order)
            break;
        removeFromList(buddy);
        MallocMetadata* combined = (buddy < block) ? buddy : block;
        combined->order += 1;
        combined->payload_size = orderToSize(combined->order) - BLOCK_BUFFER_SIZE;

        --num_allocated_blocks;
        num_allocated_bytes += _size_meta_data();
        block = combined;  
    }
    insertToFreeList(block);
    return block;
}


inline size_t orderToSize(size_t order)
{
       return (size_t)128 << order; 
}