#ifndef LRU_H
#define LRU_H

typedef struct {
    int pageFrameCount;
    int pageFaultCount;
} ReplacementAlgorithm;

typedef struct LRUNode {
    int pageNumber;
    struct LRUNode* next;
} LRUNode;

typedef struct {
    LRUNode* head;
    int currentSize;
    ReplacementAlgorithm base;
} LRU;

LRU* create_lru(int pageFrameCount);
int lru_insert(LRU* lru, int pageNumber);
void free_lru(LRU* lru);
int get_page_fault_count(LRU* lru);

#endif
