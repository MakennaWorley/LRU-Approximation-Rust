#include <stdlib.h>
#include "LRU.h"

// ReplacementAlgorithm
static void replacement_algorithm(ReplacementAlgorithm* algo, int pageFrameCount) {
    algo->pageFrameCount = pageFrameCount;
    algo->pageFaultCount = 0;
}

int get_page_fault_count(LRU* lru) {
    return lru->base.pageFaultCount;
}

// LRU
static LRUNode* search(LRU* lru, int pageNumber) {
    LRUNode* prev = NULL;
    LRUNode* curr = lru->head;

    while (curr) {
        if (curr->pageNumber == pageNumber) {
            if (prev) {
                prev->next = curr->next;
            } else {
                lru->head = curr->next;
            }
            lru->currentSize--;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

LRU* create_lru(int pageFrameCount) {
    LRU* lru = (LRU*)malloc(sizeof(LRU));
    replacement_algorithm(&lru->base, pageFrameCount);
    lru->head = NULL;
    lru->currentSize = 0;
    return lru;
}

int lru_insert(LRU* lru, int pageNumber) {
    LRUNode* found = search(lru, pageNumber);

    if (!found) {
        lru->base.pageFaultCount++;

        int evicted_page = -1;

        if (lru->currentSize == lru->base.pageFrameCount) {
            LRUNode* prev = NULL;
            LRUNode* curr = lru->head;
            while (curr && curr->next) {
                prev = curr;
                curr = curr->next;
            }

            if (curr) {
                evicted_page = curr->pageNumber;
                if (prev) {
                    prev->next = NULL;
                } else {
                    lru->head = NULL;
                }
                free(curr);
                lru->currentSize--;
            }
        }

        found = (LRUNode*)malloc(sizeof(LRUNode));
        found->pageNumber = pageNumber;

        // Insert new node at head
        found->next = lru->head;
        lru->head = found;
        lru->currentSize++;

        return evicted_page;
    }

    found->next = lru->head;
    lru->head = found;
    lru->currentSize++;

    return -1;
}

void free_lru(LRU* lru) {
    LRUNode* curr = lru->head;
    while (curr) {
        LRUNode* next = curr->next;
        free(curr);
        curr = next;
    }
    free(lru);
}
