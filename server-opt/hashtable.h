#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
struct chunk
{
    uint64_t fingerprint;
    uint64_t sketch[3];
    uint32_t length;
    struct chunk* next_chunk;
};
struct table_of_chunks
{
    uint32_t table_size;
    struct chunk* chunks;
};
struct table_of_chunks* initialize_table_of_chunks(uint32_t given_table_size);

void free_table_of_chunks(struct table_of_chunks* table);

int hash_of_fingerprint(uint64_t fp);

struct chunk* insert_fingerprint_in_table(struct table_of_chunks* table, uint64_t fp, uint64_t sf[],uint32_t len);

struct chunk* find_similar_chunk(struct table_of_chunks* table,uint64_t fp, uint64_t sf[]);

#endif
