// Compile with:
// gcc -o memalloc.so -fPIC -shared memalloc.c


#include <pthread.h>
#include <unistd.h>
#include <string.h>

// PADDING is 16b
typedef char PADDING[16];

// Make sure the hearer is always 16b
union header {
    PADDING padding;

    // Actual header
	struct {
		size_t size;
		unsigned is_free;
		union header *next;
	} s;
};

// Header typ
typedef union header header_t;

// If its locked, a malloc function is running
// To prevent threads from accesing memory at the same time
pthread_mutex_t memory_lock;

// Head and tail of the linked list (of the heap we create)
header_t *head, *tail;


// Gets a free block from the linked list (if one is free)
header_t *get_free_block_from_linked_list(size_t size)
{
    // Curr points where head points
    header_t *curr = head;

    while (curr)
    {
        // If we find a free block and suitable size
        if (curr->s.is_free && curr->s.size >= size)
        {
            return curr;
        }

        // Continue the search
        curr = curr->s.next;
    }

    // If we dont find any space
    return NULL;
}


void *malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;
    if (!size)
    {
        return NULL;
    }

    // Lock because there is action on memory
    pthread_mutex_lock(&memory_lock);

    header = get_free_block_from_linked_list(size);

    // If we found a free space in the linked list
    if (header)
    {
        // Mark the block as not free
        header->s.is_free = 0;

        // Release the global lock
        pthread_mutex_unlock(&memory_lock);

        // Return the block without the header
        return (void*)(header + 1);
    }

    // Continue if we didnt find a free block
    total_size = sizeof(header_t) + size;

    // Extend the heap by total_size
    block = sbrk(total_size);

    // Handle the case where we cant get memory
    if (block == (void*) -1)
    {
        pthread_mutex_unlock(&memory_lock);
        return NULL;
    }

    // Handle the header
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;

    // Handle the linked list
    if (!head)
    {
        head = header;
    }

    if (tail)
    {
        tail->s.next = header;
    }
    tail = header;

    // Release the lock
    pthread_mutex_unlock(&memory_lock);

    // Return the memory block
    return (void*)(header + 1);
}


void free(void *block)
{
    header_t *header, *tmp;
    void *heap_end;

    if (!block)
    {
        return;
    }

    // Lock
    pthread_mutex_lock(&memory_lock);

    // Get the header
    header = (header_t*)block - 1;

    heap_end = sbrk(0);

    // Use char* casting to make arithmetic
    // If the block is at the end of the heap
    // Here we compare memory addresses
    if((char*)block + header->s.size == heap_end)
    {

        // Recalculate head and tail
        if (head == tail)
        {
            // If there is just one element in the linked list
            head = tail = NULL;
        }
        else
        {
            // Remove the last element of the linked list
            tmp = head;
            while (tmp)
            {
                if (tmp->s.next == tail)
                {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }

        // Release the header and actual block back to OS (as its at the end of the heap)
        sbrk(0 - sizeof(header_t) - header->s.size);
        
        // Release lock
        pthread_mutex_unlock(&memory_lock);
        return;
    }

    // If the block is not at the end of the heap, free it this way
    header->s.is_free = 1;
    pthread_mutex_unlock(&memory_lock);
}


void *calloc(size_t num, size_t size)
{
    size_t total_size;
    void *block;

    if (!num || !size)
    {
        return NULL;
    }

    // Find the size to malloc
    total_size = num * size;

    // Check multiplication overflow
    if (size != total_size / num)
    {
        return NULL;
    }

    // Get the memory block
    block = malloc(total_size);

    if (!block)
    {
        return NULL;
    }

    // memset(block, 0, total_size);

    // Manually memset
    for (size_t i = 0; i < total_size; i++)
    {
        // Set all the values to 0 (as calloc does)
        ((char*)block)[i] = 0;
    }

    return block;
}


void *realloc(void *block, size_t size)
{
    header_t *header;
    void *reallocated_block;

    if (!block || !size)
    {
        return malloc(size);
    }

    // Get the header
    header = (header_t*)block - 1;

    // If previous size is greater or equal to the new size
    if (header->s.size >= size)
    {
        return block;
    }

    // Allocate new memory for the block
    reallocated_block = malloc(size);

    if (reallocated_block)
    {
        // Copy the data
        // memcpy(reallocated_block, block, header->s.size);
        for (size_t i = 0; i < header->s.size; i++)
        {
            ((char*)reallocated_block)[i] = ((char*)block)[i];
        }
        free(block);
    }

    return reallocated_block;
}