/**
 * Demonstration C program illustrating how to perform file I/O for vm assignment.
 *
 * Input file contains logical addresses
 *
 * Backing Store represents the file being read from disk (the backing store.)
 *
 * We need to perform random input from the backing store using fseek() and fread()
 *
 * This program performs nothing of meaning, rather it is intended to illustrate the basics
 * of I/O for the vm assignment. Using this I/O, you will need to make the necessary adjustments
 * that implement the virtual memory manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "LRU.h"

// number of characters to read for each line from input file
#define BUFFER_SIZE         10

// number of bytes to read
#define CHUNK               256

#define FRAME_COUNT         128
#define PAGE_TABLE_SIZE     256
#define TLB_SIZE            16

FILE    *address_file;
FILE    *backing_store;

// how we store reads from input file
char    address[BUFFER_SIZE];

int     logical_address;

// the buffer containing reads from backing store
signed char     buffer[CHUNK];

// the value of the byte (signed char) in memory
signed char     value;

int page_table[PAGE_TABLE_SIZE];
int page_table_valid[PAGE_TABLE_SIZE];
signed char physical_memory[FRAME_COUNT][CHUNK];
int next_free_frame = 0;
int tlb_page_numbers[TLB_SIZE];
int tlb_frame_numbers[TLB_SIZE];
int tlb_index = 0;

LRU *lru;

void initialize_page_table() {
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i] = -1;
        page_table_valid[i] = 0;
    }
}

void initialize_tlb() {
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb_page_numbers[i] = -1;
        tlb_frame_numbers[i] = -1;
    }
}

int search_tlb(int page_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb_page_numbers[i] == page_number) {
            return tlb_frame_numbers[i];
        }
    }
    return -1;
}

void add_to_tlb(int page_number, int frame_number) {
    tlb_page_numbers[tlb_index] = page_number;
    tlb_frame_numbers[tlb_index] = frame_number;
    tlb_index = (tlb_index + 1) % TLB_SIZE;
}

int main(int argc, char *argv[])
{
int i = 0;

    // perform basic error checking
    if (argc != 3) {
        fprintf(stderr,"Usage: ./vm [backing store] [input file]\n");
        return -1;
    }

    // open the file containing the backing store
    backing_store = fopen(argv[1], "rb");
    
    if (backing_store == NULL) { 
        fprintf(stderr, "Error opening %s\n",argv[1]);
        return -1;
    }

    // open the file containing the logical addresses
    address_file = fopen(argv[2], "r");

    if (address_file == NULL) {
        fprintf(stderr, "Error opening %s\n",argv[2]);
        return -1;
    }

    initialize_page_table();
    lru = create_lru(FRAME_COUNT);
    initialize_tlb();

    int translated_count = 0;
    int tlb_hits = 0;

    // read through the input file and output each logical address
    while ( fgets(address, BUFFER_SIZE, address_file) != NULL) {
        logical_address = atoi(address);

        /*// first seek to byte CHUNK in the backing store
        // SEEK_SET in fseek() seeks from the beginning of the file
        if (fseek(backing_store, CHUNK * i, SEEK_SET) != 0) {
            fprintf(stderr, "Error seeking in backing store\n");
            return -1;
        }

        // now read CHUNK bytes from the backing store to the buffer
        if (fread(buffer, sizeof(signed char), CHUNK, backing_store) == 0) {
            fprintf(stderr, "Error reading from backing store\n");
            return -1;
        }

        // arbitrarily retrieve the 50th byte from the buffer
        value = buffer[50];*/

        int page_number = (logical_address >> 8) & 0xFF;
        int offset = logical_address & 0xFF;
        int frame_number = search_tlb(page_number);

        if (frame_number != -1) {
            tlb_hits++;
        } else {
            if (!page_table_valid[page_number]) {
                fseek(backing_store, page_number * CHUNK, SEEK_SET);
                fread(buffer, sizeof(signed char), CHUNK, backing_store);

                int evicted_page = lru_insert(lru, page_number);

                if (next_free_frame < FRAME_COUNT) {
                    frame_number = next_free_frame++;
                } else {
                    frame_number = page_table[evicted_page];
                    page_table_valid[evicted_page] = 0;
                    page_table[evicted_page] = -1;
                }

                memcpy(physical_memory[frame_number], buffer, CHUNK);
                page_table[page_number] = frame_number;
                page_table_valid[page_number] = 1;
            }

            frame_number = page_table[page_number];
            add_to_tlb(page_number, frame_number);
        }
        value = physical_memory[frame_number][offset];
        int physical_address = frame_number * CHUNK + offset;

        printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, value);
        translated_count++;
    }

    int page_faults = get_page_fault_count(lru);
    printf("Number of Translated Addresses = %d\n", translated_count);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", (float)page_faults / translated_count);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", (float)tlb_hits / translated_count);


    fclose(address_file);
    fclose(backing_store);
    free_lru(lru);

    return 0;
}

