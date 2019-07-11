#include <stdio.h>
#include <stdlib.h>

/* access node of generic linked list */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({          \
	const typeof(((type *)0)->member)*__mptr = (ptr);    \
		     (type *)((char *)__mptr - offsetof(type, member)); })

// generic linked list
struct list_head
{
    struct list_head* next;
    struct list_head* prev;
};

// internal insert function for generic linked list
void __list_add(struct list_head* n, struct list_head* prev, struct list_head* next)
{
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;
}

// insert front of generic linked list
void list_add_front(struct list_head* n, struct list_head* head)
{
    __list_add(n, head, head->next);
}

void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}



/* hash function */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL
unsigned int hash_32(unsigned int val=100, unsigned int bits =3)
{
    unsigned int hash = val * GOLDEN_RATIO_PRIME_32;
    return hash >> (32 - bits); 
}

typedef unsigned char  __attribute__((__may_alias__))  __u8_alias_t;
typedef unsigned short __attribute__((__may_alias__)) __u16_alias_t;
typedef unsigned int __attribute__((__may_alias__)) __u32_alias_t;
typedef unsigned long long __attribute__((__may_alias__)) __u64_alias_t;

/* Optimization barrier */
/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")

static __always_inline void __write_once_size(volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(volatile  __u8_alias_t *) p = *(__u8_alias_t  *) res; break;
	case 2: *(volatile __u16_alias_t *) p = *(__u16_alias_t *) res; break;
	case 4: *(volatile __u32_alias_t *) p = *(__u32_alias_t *) res; break;
	case 8: *(volatile __u64_alias_t *) p = *(__u64_alias_t *) res; break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

#define WRITE_ONCE(x, val)				\
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__val = (val) }; 			\
	__write_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})


#define POISON_POINTER_DELTA 0
#define LIST_POISON1  ((void *) 0x100 + POISON_POINTER_DELTA)
#define LIST_POISON2  ((void *) 0x200 + POISON_POINTER_DELTA)

// head for hash list
struct hlist_head
{
    struct hlist_node * first;
};

// node for hash list
struct hlist_node
{
    struct hlist_node* next;
    struct hlist_node** pprev;
};

void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;

	WRITE_ONCE(*pprev, next);
	if (next)
		next->pprev = pprev;
}

void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = (hlist_node*)LIST_POISON1;
	n->pprev = (hlist_node**)LIST_POISON2;
}



// insert front hash node
void hlist_add_head(struct hlist_node* n, struct hlist_head* h)
{
    struct hlist_node* first = h->first;
    n->next = first;
    if(first){
        first->pprev = &n->next;
    }
    h->first = n;
    n->pprev = &h->first;
}



//--------------------------------------------------------------------------------------

// user defined data structure
typedef struct page
{
    int pfn; //page frame number
    int data;
    struct hlist_node hnode; //hash list
    struct list_head list; //lru list
}PAGE;

#define HASH_MAX (8)
// search hash list
PAGE* search_page(struct hlist_head* heads, int pfn)
{
    int i;
    struct hlist_node* temp={0,};

    static PAGE* page;
    //bucket list 총 수만큼 돌아야 하므로 HASH_MAX
    for(i=0; i< HASH_MAX; ++i){
        for(temp = heads[i].first; temp; temp=temp->next){
            page = container_of(temp, PAGE, hnode);
            if(pfn == page->pfn){
                printf("data = %d\n", page->data);
                return page;
            }
        }
    }

    return NULL;

}

bool display_hlist(struct hlist_head* heads)
{
    int i;
    struct hlist_node* temp;
    PAGE* page;
    printf("[HASH TABLE]\n");
    for(i=0; i< HASH_MAX; ++i){
        printf("[[%d]]", i);
        for(temp = heads[i].first; temp; temp=temp->next){
            page = container_of(temp, PAGE, hnode);
            printf("<->[%d]", page->data);
        }
        printf("\n");
    }

}

void display_list(struct list_head* head)
{
    PAGE* page;

    printf("[LRU LIST]\n");
    printf("[lru]");
    for(struct list_head* temp= head->next; temp != head; temp=temp->next){
        page = container_of(temp, PAGE, list);
        printf("<->[%d]", page->data);
    }
    printf("\n");
}

int main()
{
    PAGE pages[40]={0,0,};
    struct hlist_head hlistheads[HASH_MAX]={0,};
    struct list_head listhead={&listhead, &listhead};

    for(int i=0; i<40; ++i){
        pages[i].pfn = i;
        pages[i].data = i+1000;
    }

    int cntPage=0;
    while(getchar()){
        int pfn = rand()%40; // 0 ~ 39
        printf("pfn = %d\n", pfn);
        
        PAGE* page=NULL;
        page = search_page(hlistheads, pfn);

        if( page != NULL){
            list_del(&page->list);
        }else{
            cntPage++;
            if(cntPage >= 20){
                cntPage--;
                printf("Pages are occupied, ");
                page = container_of(listhead.prev, PAGE, list);
                printf("Removed pfn = %d\n", page->pfn);
                list_del(listhead.prev);
                hlist_del(&page->hnode);
            }

            printf("New page, ");
            printf("Add to hash list, ");
            hlist_add_head(&pages[pfn].hnode, &hlistheads[hash_32(pages[pfn].data ,3)]); // insert hash list
            
        }

        list_add_front(&pages[pfn].list, &listhead); //insert lru list
        
        printf("Num of pages = %d\n", cntPage);
        display_hlist(hlistheads);
        printf("\n");
        display_list(&listhead);
    }

}