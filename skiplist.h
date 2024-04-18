#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <stdbool.h>
#include <omp.h>

#define MAX_LEVEL 10
#define MAX_INT 2147483647

// Global variables
extern double cpu_time, 
              insert_start, insert_end, 
              search_start, search_end, 
              delete_start, delete_end,
              par_insert_start, par_insert_end, 
              par_search_start, par_search_end, 
              par_delete_start, par_delete_end;
extern omp_lock_t coarse_grained_lock;

// Skiplist structures for sequential and coarse-grained lock versions
typedef struct Node {
    int val;                            // each node has a val as key
    struct Node *right, *down;          // 2d singly linked
} Node;

typedef struct Skiplist {
    Node* head;                         // the top and first node of the skip list
} Skiplist;

// Skiplist structures for fine-grained lock version
typedef struct FGL_Node {
    int val;
    struct FGL_Node *right, *down;
    omp_lock_t lock;                    // added a lock for each FGL_Node
} FGL_Node;

typedef struct FGL_Skiplist {
    FGL_Node* head;
} FGL_Skiplist;

// Functions:

// Sequential
Skiplist* skiplist_init();
bool Search(Skiplist* sl, int num);
void Insert(Skiplist* sl, int num);
bool Delete(Skiplist* sl, int num);

// Coarse_grained Lock
void CGL_Insert(Skiplist* sl, int num);
bool CGL_Delete(Skiplist* sl, int num);

// Fine_grained Lock
FGL_Skiplist* fgl_skiplist_init();
bool FGL_Search(FGL_Skiplist* sl, int num);
void FGL_Insert(FGL_Skiplist* sl, int num);
bool FGL_Delete(FGL_Skiplist* sl, int num);

// Utilities
void skiplistFree(Skiplist* sl);
void FGL_skiplistFree(FGL_Skiplist* sl);

#endif