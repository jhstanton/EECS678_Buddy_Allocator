/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
				     + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...)				\
  fprintf(stderr, "%s(), %s:%d: " fmt,			\
	  __func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
  struct list_head list;
  /* TODO: DECLARE NECESSARY MEMBER VARIABLES */
  uint8_t *location; // points to start byte in memory
  size_t index;
  uint8_t order;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1]; // lists of free space, actual indexes are [MIN_ORDER - 1, MAX_ORDER]

/* memory area */
uint8_t g_memory[1<<MAX_ORDER]; // memory in bytes, pointers should point to the first byte of allocated memory

/* page structures */
// actually allocated pages? this could be a way to build up large allocations from minimumly sized allocations?
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE]; 

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
  int i;
  int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
  for (i = 0; i < n_pages; i++) {
    /* TODO: INITIALIZE PAGE STRUCTURES */
    g_pages[i].index    = i;
    g_pages[i].location = PAGE_TO_ADDR(i);
    g_pages[i].order    = -1;
  }

  /* initialize freelist */
  for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
    INIT_LIST_HEAD(&free_area[i]);
  }

  /* add the entire memory as a freeblock */
  list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
  // desired order is the ordered required to allocate
  // upper_order is the next minimum size that must be broken down
  int i, desired_order = -1, upper_order = -1;
  page_t *allocation, *buddy, *temp;

  // find both bounds on orders: target and current available space
  for(i = MIN_ORDER; i <= MAX_ORDER; i++){
    
    if (desired_order < MIN_ORDER && size <= (1 << i)){
      desired_order = i;
    }

    if(desired_order >= MIN_ORDER && !list_empty(&free_area[i])){
      upper_order = i;
      break; // found both orders, done here
    }
  }

  // step down until upper_order is split up into the desired_order - only entered if
  // upper_order != desired_order currently!
  // check for off-by-1 error here, should this be > or >=?
  for(i = upper_order; i > desired_order; i--){
    temp = list_entry(&free_area[i], page_t, list);
    list_del(&temp->list); // free up the larger block - we're splitting it in two
    allocation = temp; // still uses the same first byte //ADDR_TO_PAGE(temp->location);
    allocation->order = i;
    // using i - 1 because we're splitting an upper order to two lower orders
    buddy      = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(allocation->location, (i - 1)))];
    buddy->order = i;
    list_add(&buddy->list, &free_area[ i - 1 ]);
  }

  if(allocation == NULL){ // never entered for loop, no extra memory manipulation necessary!
    allocation = list_entry(&free_area[desired_order], page_t, list);
    list_del(&allocation->list);
    allocation->order = desired_order;
  }
		       
  
  return allocation->location;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
  /* TODO: IMPLEMENT THIS FUNCTION */
  //  size_t i, buddy_index, page_index = ADDR_TO_PAGE(addr);
  
  
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
  int o;
  for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
    struct list_head *pos;
    int cnt = 0;
    list_for_each(pos, &free_area[o]) {
      cnt++;
    }
    printf("%d:%dK ", cnt, (1<<o)/1024);
  }
  printf("\n");
}
