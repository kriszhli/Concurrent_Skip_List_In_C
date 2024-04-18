# Parallel_Skip_List_In_C
Explore the lock-based synchronization mechanism in the implementation of a parallel skip list data structure

To use the provided test file:
1. Compiler:        module load gcc-12.2
2. Compilation:     gcc-12.2 -o skiplist_test skiplist_test.c skiplist.c -fopenmp
3. Usage:           ./skiplist_test

To use the library in your own file:
1. Add skiplist.c and skiplist.h to your directory
2. Add #include "skiplist.h" to your .c file
3. gcc-12.2 -o [your_filename] [your_filename.c] skiplist.c -fopenmp
4. ./[your_filename]

Functions (check skiplist.c for definitions): 
Skiplist* skiplist_init(); 
bool Search(Skiplist* sl, int num); 
void Insert(Skiplist* sl, int num); 
bool Delete(Skiplist* sl, int num); 
void CGL_Insert(Skiplist* sl, int num); 
bool CGL_Delete(Skiplist* sl, int num); 
FGL_Skiplist* fgl_skiplist_init(); 
bool FGL_Search(FGL_Skiplist* sl, int num); 
void FGL_Insert(FGL_Skiplist* sl, int num); 
bool FGL_Delete(FGL_Skiplist* sl, int num); 
void skiplistFree(Skiplist* sl); 
void FGL_skiplistFree(FGL_Skiplist* sl);
