#define SYSCALL_FAILED(POINTER) (((long int) POINTER) == -1)

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
