#include "hashtable.h"
/*
 *initialize the table of chunks with given table size
 */
struct table_of_chunks* initialize_table_of_chunks(uint32_t given_table_size)
{
	struct table_of_chunks* table;
	
	table = (struct table_of_chunks*)malloc(sizeof(struct table_of_chunks));
	table->table_size = given_table_size;
	table->chunks = (struct chunk*)malloc(table->table_size*sizeof(struct chunk));
	
	for(int i = 0; i < table->table_size ; i++)
	{
		table->chunks[i].fingerprint=0;
        for(int j = 0;j < 3; j++)
            table->chunks[i].sketch[j] = 0;
		table->chunks[i].next_chunk=NULL;
	}
	return table;
 }

/*
 *free table of chunks
 */

void free_table_of_chunks(struct table_of_chunks* table)
{
    if(table != NULL)
    {
       for(uint32_t i = 0;i <table->table_size ;i++)
      {
        struct chunk* cur_chunk = table->chunks[i].next_chunk;
        struct chunk* pre_chunk = cur_chunk;
        while(cur_chunk != NULL)
        {
            pre_chunk = cur_chunk;
            cur_chunk = cur_chunk->next_chunk;
            free(pre_chunk);
        }
      }
    free(table->chunks);
    free(table);
    }
}

/*
 *calculate location of fingerprint
 */

int hash_of_fingerprint(uint64_t fp)
{
    return fp % 113;
}

/*
 *traverse and compare fingerprint,if not exist, insert it in table 
 */

struct chunk* insert_fingerprint_in_table(struct table_of_chunks* table, uint64_t fp, uint64_t sf[], uint32_t len)
{
    int pos = hash_of_fingerprint(fp);
    struct chunk* temp = table->chunks[pos].next_chunk;
    while(temp && temp->fingerprint != fp)
        temp = temp->next_chunk;
    if(temp == NULL)
    {
       struct chunk* new_chunk = (struct chunk*)malloc(sizeof(struct chunk));
       new_chunk->fingerprint = fp;
       for(int i = 0; i < 3; i++)
          new_chunk->sketch[i] = sf[i];
       new_chunk->next_chunk  = table->chunks[pos].next_chunk;
       new_chunk->length = len;
       table->chunks[pos].next_chunk = new_chunk;
       return new_chunk;
    }
    return NULL;
}
struct chunk* find_similar_chunk(struct table_of_chunks* table, uint64_t fp, uint64_t sf[])
{
    struct chunk* similar_chunk = NULL;
    int match = 0;
    for(int i = 0;i < table->table_size && match < 3; i++)
    {
        struct chunk* temp = table->chunks[i].next_chunk;
        while(temp != NULL)
        {
            int cnt = 0;
            for(int j = 0; j < 3 && temp->fingerprint != fp; j++)
               for(int k = 0; k < 3; k++)
                    if(temp->sketch[j] == sf[k])
                    { cnt++;break; }
            if(cnt > match)
            {
                match = cnt;
                similar_chunk = temp;
            }
            temp = temp->next_chunk;
        }
    }
    return similar_chunk;
}
