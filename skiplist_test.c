// Compilation:     gcc-12 -o skiplist_test skiplist_test.c skiplist.c -fopenmp
// Usage:           ./skiplist_test

#include "skiplist.h"                       // include "skiplist.h" in your c program to use
#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 8                       // change this to test the effect of the number of threads
#define TEST_SIZE 100000                    // change this to test the effect of the size of the skip list

// declace testing local variable
omp_lock_t coarse_grained_lock;
double  cpu_time, 
        insert_start, insert_end,
        search_start, search_end,
        delete_start, delete_end,
        par_insert_start, par_insert_end,
        par_search_start, par_search_end,
        par_delete_start, par_delete_end;

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void shuffle(int arr[], int n) {
    srand(11);
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&arr[i], &arr[j]);
    }
}

int main() {
    Skiplist* sl_10 = skiplist_init();
    omp_set_num_threads(NUM_THREADS);
    omp_init_lock(&coarse_grained_lock);
    #pragma omp parallel for
    for (int i = 1; i <= 10; i++) {
        CGL_Insert(sl_10, i);
    }
    omp_destroy_lock(&coarse_grained_lock);
    printf("\n============================================================\n");
    printf("  Real Time Skip List Demonstration (Coarse-grained Lock) \n");
    printf("============================================================\n");
    printf(" 1. Insert the sequence [1, 10] into a Skip List with max level of %d: \n", MAX_LEVEL);
    Node* temp = sl_10->head;
    for (int level = MAX_LEVEL - 1; level >= 0; level--) {
        printf("      Level %d: ", level + 1);
        Node* current = temp;
        while (current) {
            if (current->val != -MAX_INT) {
                printf("%d -> ", current->val);
            }
            current = current->right;
        }
        printf("NULL\n");
        temp = temp->down;
    }

    printf(" 2. Delete 3, 6, and 7 from the Skip List:\n");
    Delete(sl_10, 3);
    Delete(sl_10, 6);
    Delete(sl_10, 7);
    Node* temp2 = sl_10->head;
    for (int level = MAX_LEVEL - 1; level >= 0; level--) {
        printf("      Level %d: ", level + 1);
        Node* current = temp2;
        while (current) {
            if (current->val != -MAX_INT) {
                printf("%d -> ", current->val);
            }
            current = current->right;
        }
        printf("NULL\n");
        temp2 = temp2->down;
    }

    printf(" 3. Search for 5 and 6 in the Skip List:\n");
    printf("      Searching for 5 ... Result is: %s\n", Search(sl_10, 5) ? "Found" : "Not found");
    printf("      Searching for 6 ... Result is: %s\n", Search(sl_10, 6) ? "Found" : "Not found");

    printf(" 3. Insert 1 and 6; then search for 5 and 6 again:\n");
    CGL_Insert(sl_10, 1);
    CGL_Insert(sl_10, 6);
    Node* temp3 = sl_10->head;
    for (int level = MAX_LEVEL - 1; level >= 0; level--) {
        printf("      Level %d: ", level + 1);
        Node* current = temp3;
        while (current) {
            if (current->val != -MAX_INT) {
                printf("%d -> ", current->val);
            }
            current = current->right;
        }
        printf("NULL\n");
        temp3 = temp3->down;
    }
    printf("      Searching for 5 ... Result is: %s\n", Search(sl_10, 5) ? "Found" : "Not found");
    printf("      Searching for 6 ... Result is: %s\n\n", Search(sl_10, 6) ? "Found" : "Not found");

    skiplistFree(sl_10);

// ======================================================================== //
// =========================== [ T E S T S ] ============================== //
// ======================================================================== //

// ======================================================================== //
// ===================== 1. O R D E R E D  A R R A Y ====================== //
// ======================================================================== //

    // ======================================================================== //
    // ====================== 1.a  S E Q U E N T I A L ======================== //
    // ======================================================================== //
    // ==== Sequential insertion of the ordered array [1, 100000] ====//

    printf("============================================================\n");
    printf("    Real time testing an ordered array of length %d\n", TEST_SIZE);
    printf("============================================================\n");
    printf("Sequential:\n");

    // Initiate the skip list for the ordered array [1, 100000]
    Skiplist* sl_ord = skiplist_init();
    insert_start = omp_get_wtime();
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Insert(sl_ord, i);
    }
    insert_end = omp_get_wtime();
    printf("-- Insertion time: %.4f ms\n", (insert_end - insert_start) * 1000);

    // ===== Sequential searching of the ordered array [1, 100000] ==== //

    search_start = omp_get_wtime();
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Search(sl_ord, i);
    }
    search_end = omp_get_wtime();
    printf("-- Search time: %.4f ms\n", (search_end - search_start) * 1000);

    // ===== Sequential deletion of the ordered array [1, 100000] ===== //

    delete_start = omp_get_wtime();
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Delete(sl_ord, i);
    }
    delete_end = omp_get_wtime();
    printf("-- Deletion time: %.4f ms\n", (delete_end - delete_start) * 1000);

    skiplistFree(sl_ord);

    // ======================================================================== //
    // ===================== 1.b  G L O B A L   L O C K ======================= //
    // ======================================================================== //

    // ===== Coarse-grained lock insertion of the ordered array [1, 100000] ====== //
    
    printf("Coarse-grained lock with %d threads:\n", NUM_THREADS);

    Skiplist* sl_ord_gl = skiplist_init();

    par_insert_start = omp_get_wtime();
    omp_init_lock(&coarse_grained_lock);
    #pragma omp parallel for
    for (int i = 1; i <= 1000; i++)
    {
        CGL_Insert(sl_ord_gl, i);
    }
    omp_destroy_lock(&coarse_grained_lock);
    par_insert_end = omp_get_wtime();
    printf("-- Insertion time: %.4f ms; speedup = %.4f\n", (par_insert_end - par_insert_start) * 1000, (insert_end - insert_start)/(par_insert_end - par_insert_start));

    // ===== parallel searching of the ordered array [1, 100000] ======== //

    par_search_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Search(sl_ord_gl, i);
    }
    par_search_end = omp_get_wtime();
    printf("-- Search time (no lock needed): %.4f ms; speedup = %.4f\n", (par_search_end - par_search_start) * 1000, (search_end - search_start)/(par_search_end - par_search_start));

    // ===== Coarse-grained lock deletion of the ordered array [1, 100000] ====== //

    par_delete_start = omp_get_wtime();
    omp_init_lock(&coarse_grained_lock);
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        CGL_Delete(sl_ord_gl, i);
    }
    omp_destroy_lock(&coarse_grained_lock);
    par_delete_end = omp_get_wtime();
    printf("-- Deletion time: %.4f ms; speedup = %.4f\n", (par_delete_end - par_delete_start) * 1000, (delete_end - delete_start)/(par_delete_end - par_delete_start));
    skiplistFree(sl_ord_gl);

    // ======================================================================== //
    // ================ 1.c  F I N E - G R A I N E D   L O C K ================ //
    // ======================================================================== //

    // ===== Fine-grained lock insertion of the ordered array [1, 100000] ===== //
    
    printf("Fine-grained lock with %d threads:\n", NUM_THREADS);

    FGL_Skiplist* sl_ord_fgl = fgl_skiplist_init();

    par_insert_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= 1000; i++)
    {
        FGL_Insert(sl_ord_fgl, i);
    }
    par_insert_end = omp_get_wtime();

    printf("-- Insertion time: %.4f ms; speedup = %.4f\n", (par_insert_end - par_insert_start) * 1000, (insert_end - insert_start)/(par_insert_end - par_insert_start));

    // ===== parallel searching of the ordered array [1, 100000] ======== //

    par_search_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        FGL_Search(sl_ord_fgl, i);
    }
    par_search_end = omp_get_wtime();
    printf("-- Search time (no lock needed): %.4f ms; speedup = %.4f\n", (par_search_end - par_search_start) * 1000, (search_end - search_start)/(par_search_end - par_search_start));

    // ===== Fine-grained lock deletion of the ordered array [1, 100000] ====== //

    par_delete_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        FGL_Delete(sl_ord_fgl, i);
    }
    par_delete_end = omp_get_wtime();
    printf("-- Deletion time: %.4f ms; speedup = %.4f\n", (par_delete_end - par_delete_start) * 1000, (delete_end - delete_start)/(par_delete_end - par_delete_start));
    
    FGL_skiplistFree(sl_ord_fgl);

// ======================================================================== //
// ===================== 2. R A N D O M  A R R A Y ======================== //
// ======================================================================== //

    int random_array[TEST_SIZE];
    for (int i = 0; i < TEST_SIZE; i++) {
        random_array[i] = i + 1;
    }
    shuffle(random_array, TEST_SIZE);
    printf("\n");

    // ======================================================================== //
    // ====================== 2.a  S E Q U E N T I A L ======================== //
    // ======================================================================== //
    // ==== Sequential insertion of the random/shuffled array [1, 100000] ====//

    printf("============================================================\n");
    printf("    Real time testing a random array of length %d\n", TEST_SIZE);
    printf("============================================================\n");
    printf("Sequential:\n");

    // Initiate the skip list for the ordered array [1, 100000]
    Skiplist* sl_rand = skiplist_init();
    insert_start = omp_get_wtime();
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Insert(sl_rand, random_array[i]);
    }
    insert_end = omp_get_wtime();
    printf("-- Insertion time: %.4f ms\n", (insert_end - insert_start) * 1000);

    // ===== Sequential searching of the ordered array [1, 100000] ==== //

    search_start = omp_get_wtime();
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Search(sl_rand, random_array[i]);
    }
    search_end = omp_get_wtime();
    printf("-- Search time: %.4f ms\n", (search_end - search_start) * 1000);

    // ===== Sequential deletion of the ordered array [1, 100000] ===== //

    delete_start = omp_get_wtime();
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Delete(sl_rand, random_array[i]);
    }
    delete_end = omp_get_wtime();
    printf("-- Deletion time: %.4f ms\n", (delete_end - delete_start) * 1000);

    skiplistFree(sl_rand);

    // ======================================================================== //
    // ===================== 2.b  G L O B A L   L O C K ======================= //
    // ======================================================================== //

    // ===== Coarse-grained lock insertion of the ordered array [1, 100000] ====== //
    
    printf("Coarse-grained lock with %d threads:\n", NUM_THREADS);

    Skiplist* sl_rand_gl = skiplist_init();

    par_insert_start = omp_get_wtime();
    omp_init_lock(&coarse_grained_lock);
    #pragma omp parallel for
    for (int i = 1; i <= 1000; i++)
    {
        CGL_Insert(sl_rand_gl, random_array[i]);
    }
    omp_destroy_lock(&coarse_grained_lock);
    par_insert_end = omp_get_wtime();

    printf("-- Insertion time: %.4f ms; speedup = %.4f\n", (par_insert_end - par_insert_start) * 1000, (insert_end - insert_start)/(par_insert_end - par_insert_start));

    // ===== parallel searching of the ordered array [1, 100000] ======== //

    par_search_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        Search(sl_rand_gl, random_array[i]);
    }
    par_search_end = omp_get_wtime();
    printf("-- Search time (no lock needed): %.4f ms; speedup = %.4f\n", (par_search_end - par_search_start) * 1000, (search_end - search_start)/(par_search_end - par_search_start));

    // ===== Coarse-grained lock deletion of the ordered array [1, 100000] ====== //

    par_delete_start = omp_get_wtime();
    omp_init_lock(&coarse_grained_lock);
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        CGL_Delete(sl_rand_gl, random_array[i]);
    }
    omp_destroy_lock(&coarse_grained_lock);
    par_delete_end = omp_get_wtime();
    printf("-- Deletion time: %.4f ms; speedup = %.4f\n", (par_delete_end - par_delete_start) * 1000, (delete_end - delete_start)/(par_delete_end - par_delete_start));
    skiplistFree(sl_rand_gl);

    // ======================================================================== //
    // ================ 2.c  F I N E - G R A I N E D   L O C K ================ //
    // ======================================================================== //

    // ===== Fine-grained lock insertion of the ordered array [1, 100000] ===== //
    
    printf("Fine-grained lock with %d threads:\n", NUM_THREADS);

    FGL_Skiplist* sl_rand_fgl = fgl_skiplist_init();

    par_insert_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= 1000; i++)
    {
        FGL_Insert(sl_rand_fgl, random_array[i]);
    }
    par_insert_end = omp_get_wtime();

    printf("-- Insertion time: %.4f ms; speedup = %.4f\n", (par_insert_end - par_insert_start) * 1000, (insert_end - insert_start)/(par_insert_end - par_insert_start));

    // ===== parallel searching of the ordered array [1, 100000] ======== //

    par_search_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        FGL_Search(sl_rand_fgl, random_array[i]);
    }
    par_search_end = omp_get_wtime();
    printf("-- Search time (no lock needed): %.4f ms; speedup = %.4f\n", (par_search_end - par_search_start) * 1000, (search_end - search_start)/(par_search_end - par_search_start));

    // ===== Fine-grained lock deletion of the ordered array [1, 100000] ====== //

    par_delete_start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 1; i <= TEST_SIZE; i++)
    {
        FGL_Delete(sl_rand_fgl, random_array[i]);
    }
    par_delete_end = omp_get_wtime();
    printf("-- Deletion time: %.4f ms; speedup = %.4f\n\n", (par_delete_end - par_delete_start) * 1000, (delete_end - delete_start)/(par_delete_end - par_delete_start));
    FGL_skiplistFree(sl_rand_fgl);

    return 0;
}