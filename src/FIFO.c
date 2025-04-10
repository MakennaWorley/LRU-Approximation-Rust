#include <stdlib.h>
#include "FIFO.h"

// ReplacementAlgorithm
static void replacement_algorithm(ReplacementAlgorithm* algo, int pageFrameCount) {
    algo->pageFrameCount = pageFrameCount;
    algo->pageFaultCount = 0;
}

int get_page_fault_count(FIFO* fifo) {
    return fifo->base.pageFaultCount;
}

// FIFO
static int search(FIFO* fifo, int pageNumber) {
    for (int i = 0; i < fifo->base.pageFrameCount; i++) {
        if (fifo->pageFrameList[i] == pageNumber) {
            return 1;
        }
    }
    return 0;
}

FIFO* create_fifo(int pageFrameCount) {
    FIFO* fifo = (FIFO*)malloc(sizeof(FIFO));
    replacement_algorithm(&fifo->base, pageFrameCount);
    fifo->elementCount = 0;
    fifo->nextToReplace = 0;
    fifo->pageFrameList = (int*)malloc(sizeof(int) * pageFrameCount);

    for (int i = 0; i < pageFrameCount; i++) {
        fifo->pageFrameList[i] = -1;
    }

    return fifo;
}

int insert_fifo(FIFO* fifo, int pageNumber) {
    if (!search(fifo, pageNumber)) {
        fifo->base.pageFaultCount++;
        int evicted_page = -1;

        if (fifo->elementCount < fifo->base.pageFrameCount) {
            fifo->pageFrameList[fifo->elementCount] = pageNumber;
            fifo->elementCount++;
        } else {
            evicted_page = fifo->pageFrameList[fifo->nextToReplace];
            fifo->pageFrameList[fifo->nextToReplace] = pageNumber;
            fifo->nextToReplace = (fifo->nextToReplace + 1) % fifo->base.pageFrameCount;
        }

        return evicted_page;
    }

    return -1;
}

void free_fifo(FIFO* fifo) {
    free(fifo->pageFrameList);
    free(fifo);
}
