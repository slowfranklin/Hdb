#ifndef link_list
#define link_list

typedef struct LinkList LinkList;
typedef struct LinkList Linklist;

Linklist* newLinklist();
void initLinklist(Linklist* linklist);
int pushLinklist(Linklist* linklist,void* data);
void* popLinklist(Linklist* linklist);
void freeLinklist(Linklist* linklist);
#endif