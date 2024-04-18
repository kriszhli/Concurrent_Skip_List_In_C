#include "skiplist.h"
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_LEVEL 10        // the default skiplist has 10 levels
#define MAX_INT 2147483647  // infinity as int

// Initialize a sequential/coarse-grained lock node
Node* node_init(int val) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->val = val;
    newNode->right = NULL;
    newNode->down = NULL;
    return newNode;
}

// Initialize a sequential/coarse-grained lock skip list
Skiplist* skiplist_init() {
    Skiplist* sl = (Skiplist*)malloc(sizeof(Skiplist));
    sl->head = node_init(-MAX_INT);
    return sl;
}

// Initiation for fine-grained lock node: 
FGL_Node* fgl_node_init(int val) {
    FGL_Node* newNode = (FGL_Node*)malloc(sizeof(FGL_Node));
    newNode->val = val;
    newNode->right = NULL;
    newNode->down = NULL;
    return newNode;
}

// Initiation for fine-grained lock skip list: 
FGL_Skiplist* fgl_skiplist_init() {
    FGL_Skiplist* sl = (FGL_Skiplist*)malloc(sizeof(FGL_Skiplist));
    sl->head = fgl_node_init(-MAX_INT);
    return sl;
}

// ======================================================================== //
// ============================= S E A R C H ============================== //
// ======================================================================== //

// Search is read-only so it can be used in parallel without synchronizaton
bool Search(Skiplist* sl, int num) {
    Node* temp = sl->head;
    while (temp) {
        while (temp->right && temp->right->val < num) {
            temp = temp->right;
        }
        if (temp->right && temp->right->val == num) {
            return true;
        }
        temp = temp->down;
    }
    return false;
}

// ======================================================================== //
// ========= F I N E - G R A I N E D  L O C K  S E A R C H ================ //
// ======================================================================== //

// Search is read-only so it can be used in parallel without synchronizaton
// This is modified only for the FGL_Skiplist input argument
bool FGL_Search(FGL_Skiplist* sl, int num) {
    FGL_Node* temp = sl->head;
    while (temp) {
        while (temp->right && temp->right->val < num) {
            temp = temp->right;
        }
        if (temp->right && temp->right->val == num) {
            return true;
            break;
        }
        temp = temp->down;
    }
    return false;
}

// ======================================================================== //
// ============================= I N S E R T ============================== //
// ======================================================================== //

// Create an int representing the random level of the new inserted node by flipping a coin
int rand_level() {
    int level = 1;
    while (rand() % 2 && level < MAX_LEVEL) {
        level++;
    }
    return level;
}

void Insert(Skiplist* sl, int num) {
    Node* temp = sl->head;
    Node* newNode = node_init(num);
    int currentLevel = MAX_LEVEL;
    int randLevel = rand_level();
    while (currentLevel > 0 && temp){
        // Find the correct position by moving right
        while (temp->right && temp->right->val < num) {
            temp = temp->right;
        }
        // Add the node if reached the level
        if (randLevel >= currentLevel) {
            newNode->right = temp->right;
            temp->right = newNode;
            if (currentLevel == 1) {
                newNode->down = NULL;
            }
            else {
                // Add a new level
                if (!temp->down) {
                    Node* newHead = node_init(-MAX_INT);
                    temp->down = newHead;
                }
                newNode->down = node_init(num);
            }
            // Move down
            newNode = newNode->down;
        } else {
            if (!temp->down) {
                Node* newHead = node_init(-MAX_INT);
                temp->down = newHead;
            }
        }
        temp = temp->down;
        currentLevel--;
    }
}

// ======================================================================== //
// ================= C O A R S E  L O C K  I N S E R T ==================== //
// ======================================================================== //

void CGL_Insert(Skiplist* sl, int num) {
    // Lock the whole list
    omp_set_lock(&coarse_grained_lock);
    Node* temp = sl->head;
    Node* newNode = node_init(num);
    int currentLevel = MAX_LEVEL;
    int randLevel = rand_level();
    while (currentLevel > 0 && temp){
        // Find the correct position by moving right
        while (temp->right && temp->right->val < num) {
            temp = temp->right;
        }
        // Add the node if reached the level
        if (randLevel >= currentLevel) {
            newNode->right = temp->right;
            temp->right = newNode;
            if(currentLevel==1){
                newNode->down = NULL;
            }
            else {
                // Add a new level
                if (!temp->down){
                    Node* newHead = node_init(-MAX_INT);
                    temp->down = newHead;
                }
                newNode->down = node_init(num);
            }
            newNode = newNode->down;
        }
        else {
            if (!temp->down){
                Node* newHead = node_init(-MAX_INT);
                temp->down = newHead;
            }
        }
        temp = temp->down;
        currentLevel--;
    }
    // Unlock the whole list
    omp_unset_lock(&coarse_grained_lock);
}

// ======================================================================== //
// =========== F I N E - G R A I N E D  L O C K  I N S E R T ============== //
// ======================================================================== //

void FGL_Insert(FGL_Skiplist* sl, int num) {

    FGL_Node* temp = sl->head;
    FGL_Node* newNode = fgl_node_init(num);
    int currentLevel = MAX_LEVEL;
    int randLevel = rand_level();

    // Lock the head node before traversal
    omp_set_lock(&temp->lock);

    while (currentLevel > 0 && temp){
        // Lock the current node before traversal
        omp_set_lock(&temp->lock);

        while (temp->right && temp->right->val < num) {
            // Unlock the previous node
            omp_unset_lock(&temp->lock);
            temp = temp->right;
            // Lock the new current node
            omp_set_lock(&temp->lock);
        }
        if (randLevel >= currentLevel) {
            newNode->right = temp->right;
            temp->right = newNode;

            // Unlock the current node after modification
            omp_unset_lock(&temp->lock);

            if (currentLevel==1){
                newNode->down = NULL;
            }
            else {
                if (!temp->down){
                    FGL_Node* newHead = fgl_node_init(-MAX_INT);
                    temp->down = newHead;
                }
                newNode->down = fgl_node_init(num);
            }
            newNode = newNode->down;
            }
        else {
            if (!temp->down){
                FGL_Node* newHead = fgl_node_init(-MAX_INT);
                temp->down = newHead;
            }
        }
        // Unlock the current node before moving down
        omp_unset_lock(&temp->lock);

        temp = temp->down;
        currentLevel--;
    }

}

// ======================================================================== //
// ============================= D E L E T E ============================== //
// ======================================================================== //

bool Delete(Skiplist* sl, int num) {
    bool flag = false; // use bool to return false if failed to find thus can't delete
    Node* temp = sl->head;
    while (temp) {
        while (temp->right && temp->right->val < num) {
            temp = temp->right;
        }
        if (temp->right && temp->right->val == num) {
            Node* node = temp->right; // connect the prev and next to delete current node
            temp->right = node->right;
            free(node); // free to delete
            flag = true; // return success
        }
        temp = temp->down;
    }
    return flag;
}

// ======================================================================== //
// ================= C O A R S E  L O C K  D E L E T E ==================== //
// ======================================================================== //

bool CGL_Delete(Skiplist* sl, int num) {
    // Lock the whole list
    omp_set_lock(&coarse_grained_lock);
    bool flag = false;
    Node* temp = sl->head;
    while (temp) {
        while (temp->right && temp->right->val < num) {
            temp = temp->right;
        }
        if (temp->right && temp->right->val == num) {
            Node* node = temp->right;
            temp->right = node->right;
            free(node);
            flag = true;
        }
        temp = temp->down;
    }
    omp_unset_lock(&coarse_grained_lock);
    return flag;
}

// ======================================================================== //
// =========== F I N E - G R A I N E D  L O C K  D E L E T E ============== //
// ======================================================================== //

bool FGL_Delete(FGL_Skiplist* sl, int num) {
    FGL_Node* temp = sl->head;
    bool flag = false;

    // Lock the head node before traversal
    omp_set_lock(&temp->lock);

    while (temp) {
        FGL_Node* curr = temp->right;

        // Lock the current node before traversal
        if (curr)
            omp_set_lock(&curr->lock);

        FGL_Node* prev = NULL;
        while (curr && curr->val < num) {
            // Unlock the previous node
            if (prev)
                omp_unset_lock(&prev->lock);
            prev = curr;
            curr = curr->right;
            // Lock the new current node
            if (curr)
                omp_set_lock(&curr->lock);
        }
        if (curr && curr->val == num) {
            // Unlock the previous node
            if (prev)
                omp_unset_lock(&prev->lock);

            // Remove the current node
            if (prev)
                prev->right = curr->right;
            else
                temp->right = curr->right;

            FGL_Node* node = curr;
            omp_unset_lock(&curr->lock);
            free(node);
            flag = true;
        } else {
            // Unlock the previous and current nodes
            if (prev)
                omp_unset_lock(&prev->lock);
            if (curr)
                omp_unset_lock(&curr->lock);
        }
        temp = temp->down;
        if (temp) {
            // Lock the next level node before traversal
            omp_set_lock(&temp->lock);
        }
    }

    return flag;
}

// ======================================================================== //
// ========================== U T I L I T I E S =========================== //
// ======================================================================== //

void skiplistFree(Skiplist* sl) {
    Node* temp = sl->head;
    while (temp) {
        Node* del = temp;
        temp = temp->down;
        free(del);  // free every node
    }
    free(sl);
}

void FGL_skiplistFree(FGL_Skiplist* sl) {
    FGL_Node* temp = sl->head;
    while (temp) {
        FGL_Node* del = temp;
        temp = temp->down;
        free(del); // free every node
    }
    free(sl);
}