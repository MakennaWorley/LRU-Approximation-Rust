#ifndef FIFO_H
#define FIFO_H

typedef struct {
    int pageFrameCount;
    int pageFaultCount;
} ReplacementAlgorithm;

typedef struct {
    int* pageFrameList;
    int elementCount;
    int nextToReplace;
    ReplacementAlgorithm base;
} FIFO;

FIFO* create_fifo(int pageFrameCount);
int insert_fifo(FIFO* fifo, int pageNumber);
void free_fifo(FIFO* fifo);
int get_page_fault_count(FIFO* fifo);

#endif
