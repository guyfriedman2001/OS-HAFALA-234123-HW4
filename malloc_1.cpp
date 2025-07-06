#define FAILED(POINTER) (((long int) POINTER) == -1)
#define BYTES_IN_ARCHITECTURE (4) //4 if we are in a 32 bit machine, if in 64 bit then it should be 8
#define BOOK_KEEPING_VARIABLES_AMMOUNT (4) //ammount of actual variables we would need to allocate




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
        block size (long)
              |
    next avalible block (long) //maintained only for 'free' blocks, points to the addres that was returned from the malloc call (not actual start of block)
              |
              |
          'payload' //('long aligned' blocks of size of user request)
              |
              | <- returned *ptr (actual start of free space)
    prev avalible block (long)
              |
        block size (long) //(last bit of size would be the flag for malloced/ free, as per the ATAM lecture)
      XX--other data--XX

     */
    if (size == 0 || size > 100000000){
        return nullptr;
    }
    void* start = sbrk(0);
    size_t alligned_size = ((size)+(size%BYTES_IN_ARCHITECTURE)); //allign size
    size_t actual_size =  alligned_size + BOOK_KEEPING_VARIABLES_AMMOUNT*BYTES_IN_ARCHITECTURE;
    if (FAILED(start) || FAILED(sbrk(actual_size))){
        return nullptr;
    }
    size_t actual_start = (size_t)start;
    actual_start += (2*BYTES_IN_ARCHITECTURE);
    //TODO: add the metadata

    return start;
}
