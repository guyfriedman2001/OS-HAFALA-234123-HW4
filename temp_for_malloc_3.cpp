/* experimental code below, to replace the typdefs with hard type checks */

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

/* experimental code above, to replace the typdefs with hard type checks */


/* old declarations from part 2 below */

typedef unsigned long size_t;    // FIXME: delete this line! its only for my mac
void *sbrk(payload_size_t payload_size); // FIXME: delete this line! its only for my mac

#include <cassert>
#include <string.h>

#if 0
/* typedef for clarity */
typedef void *payload_start;
typedef void *actual_block_start;
typedef size_t payload_size_t;
typedef size_t actual_size_t;
#endif

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
payload_start smalloc_helper_find_avalible(payload_size_t payload_size); // TODO: update for new implementation!
actual_block_start actually_allocate(actual_size_t actual_block_size);
inline void markFree(payload_start block); // TODO: update for new implementation!
inline void markAllocated(payload_start block); // TODO: update for new implementation! (need to check actual allocation size compared to payload size and maybe split the block)
inline payload_size_t getBlockSize(payload_start block);
inline bool isAllocated(payload_start block);
inline bool isFree(payload_start block);
inline bool isSizeValid(payload_size_t payload_size);
inline void initializeList();
inline payload_start _initBlock_MetaData(actual_block_start block, actual_size_t actual_block_size);
inline payload_start initAllocatedBlock(actual_block_start block, actual_size_t actual_block_size);
inline payload_start initFreeBlock(actual_block_start block, actual_size_t actual_block_size); // TODO: update for new implementation!
inline MallocMetadata *getMallocStruct(payload_start block);
inline size_t _size_meta_meta_data();
inline MallocMetadata *getGlobalMallocStructHeadFree(); // TODO: update for new implementation!
inline MallocMetadata *getNextMallocBlock(MallocMetadata *current_block);
inline MallocMetadata *getGlobalMallocStructTailFree(); // TODO: update for new implementation!
inline payload_start getStructsPayload(MallocMetadata *malloc_of_block);
inline void _placeBlockInFreeList(MallocMetadata *malloc_manager_of_block); // TODO: update for new implementation!
inline MallocMetadata* _firstBlockAfter(MallocMetadata *malloc_manager_of_block);

/* memory management meta data struct */
struct MallocMetadata
{
    payload_size_t payload_size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
};

/* defines for things we need to ask for in the piazza */
#define ACCOUNT_FOR__size_meta_meta_data (1)  // <- if we do not need to account for size of head_dummy, tail_dummy etc then flip this flag to 0
#define IS_OK_TO_INCLUDE_ASSERT (1)           // <- if we can not include assert, flip flag to 0.

#if IS_OK_TO_INCLUDE_ASSERT
#include <cassert>
#else //IS_OK_TO_INCLUDE_ASSERT
#define assert(expr) ((void)0) // <- if we can not include assert, this is (apperantly) a valid no-op statement.
#endif //IS_OK_TO_INCLUDE_ASSERT

/* old declarations from part 2 above */





/* new implementation for the part 3 below */

// new #defines for malloc3
#define NUM_ORDERS (10)

// new function declaration for malloc3
inline void _init_dummy_MetaData(MallocMetadata* initialise_this);
inline void initializeArrayList();

// new variables for part 3
static MallocMetadata head_dummy_array[NUM_ORDERS]; // <- all structs are not initialised and contain garbage
static MallocMetadata tail_dummy_array[NUM_ORDERS]; // <- all structs are not initialised and contain garbage

// irrelevant variables from part 2 (TODO: delte these upon migration)
static MallocMetadata head_dummy = {0, true, nullptr, nullptr};
static MallocMetadata tail_dummy = {0, true, nullptr, nullptr};

// need to update this for part 3
static bool is_list_initialized = false; // <- TODO: after we move the code to malloc_3.cpp, uodate name to be more fitting

// these stay the same as from part 2
static size_t num_allocated_blocks = 0;
static payload_size_t num_allocated_bytes = 0;

inline void initializeArrayList(){
    // use a more fitting name without changing existing usages in code
    initializeList();
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

        // now make the pointers in the correct way
        head_dummy->next = tail_dummy;
        tail_dummy->prev = head_dummy;
    }

    // now after everything is initialised, update the flag.
    is_list_initialized = true;
}

inline void _init_dummy_MetaData(MallocMetadata* initialise_this){

    // make sure no funny buisness is going on
    assert((initialise_this!=nullptr) && "function '_init_dummy_MetaData' got 'initialise_this' == nullptr.");

    //init to -> {0, true, nullptr, nullptr};
    initialise_this->payload_size = 0;
    initialise_this->is_free = true;
    initialise_this->next = nullptr;
    initialise_this->prev = nullptr;
}

inline size_t _size_meta_meta_data()
{   // <- TODO: ask in the piazza if this is needed, if not simply return 0
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

/* new implementation for the part 3 above */
