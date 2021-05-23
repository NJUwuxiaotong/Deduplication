#ifndef _FUNCTION_H_
#define _FUNCTION_H

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "SHA-256.h"

void initialize_generated(uint32_t array[][2]);

uint64_t* calculate_features(uint64_t cur_feature,uint64_t* features,uint64_t* hash_of_fp);

uint64_t* calculate_super_features(uint64_t* super_features,uint64_t* features);

uint64_t get_fp_of_block(char* buf, uint32_t len_of_buf);

char* get_chunk_from_file(char* ref,struct chunk* cur_chunk, const char* chunk);

void write_chunk_to_file(void* buf,uint32_t len_of_buf,struct chunk* cur_chunk,const char* target);

#endif
