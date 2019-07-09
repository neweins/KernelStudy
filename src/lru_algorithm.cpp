#include <stdio.h>
#include <stdlib.h>

/* generic linked list */
#define offsetof(TYPE, MEMBER)  ((size_t)&((TYPE*)0)->MEMBER)
#define container_of(ptr, type, member) ({ \
            const typeof( ((type *)0)->member ) * __mptr = (ptr);   \
            (type *)( (char*)__mptr - offsetof(type,member) ); });
// 


struct list_head
{
    struct list_head* next;
    struct list_head* prev;
};

void __list_add(struct list_head* n, struct list_head* prev, struct list_head* next)
{
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;
}

void list_add_front(struct list_head* n, struct list_head* head)
{
    __list_add(n, head, head->next);
}


/* generic hash */
struct hlist_head
{
    struct hlist_node * first;
};

struct hlist_node
{
    struct hlist_node* next;
    struct hlist_node** pprev;
};

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


//data structure
struct page
{
    int pfn; //page frame number
    int data;
    struct hlist_node hnode; //hash list
    struct list_head list; //lru list
};


int main()
{
    struct page pages[40];

    for(int i=0; i<40; ++i){
        pages[i].pfn = i;
        pages[i].data = i+1000;
    }

    while(getchar()){
        int pfn = rand()%40; // 0 ~ 39
        printf("pfn = %d", pfn);
        
    }

}