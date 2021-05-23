/*
 * rabin_polynomial.c
 *
 * Created by Joel Lawrence Tucci on 09-March-2011.
 *
 * Copyright (c) 2011 Joel Lawrence Tucci
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the project's author nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "rabin_polynomial.h"
#include "rabin_polynomial_constants.h"

uint64_t rabin_polynomial_prime = RAB_POLYNOMIAL_REM;

unsigned int rabin_sliding_window_size = RAB_POLYNOMIAL_WIN_SIZE;

unsigned int rabin_polynomial_max_block_size = RAB_MAX_BLOCK_SIZE;
unsigned int rabin_polynomial_min_block_size = RAB_MIN_BLOCK_SIZE;

unsigned int rabin_polynomial_average_block_size = RAB_POLYNOMIAL_AVG_BLOCK_SIZE;

int rabin_poly_init_completed = 0;

uint64_t *polynomial_lookup_buf;

static int initialize_rabin_polynomial(uint64_t prime, unsigned max_size, unsigned int min_size, unsigned int average_block_size);

uint64_t total_delta_bytes = 0;
uint64_t total_read_bytes = 0;
uint64_t total_compress_bytes = 0;
uint64_t total_duplicate_bytes = 0;

uint64_t features[12] = {0};
uint64_t super_features[3] = {0};
uint64_t hash_of_fp[12] = {0};
/**
 * Prints the list of rabin polynomials to the given file
 */
void print_rabin_poly_list_to_file(FILE *out_file, struct rabin_polynomial *poly)
{
  struct rabin_polynomial *cur_poly = poly;

  while (cur_poly != NULL) {
    print_rabin_poly_to_file(out_file, cur_poly, 1);
    cur_poly = cur_poly->next_polynomial;
  }
}

/**
 * Prints a given rabin polynomial to file in the format:
 * start,length hash
 */
void print_rabin_poly_to_file(FILE *out_file, struct rabin_polynomial *poly, int new_line)
{
  if (poly == NULL)
    return;

  fprintf(out_file, "%lu,%u %lu", poly->start, poly->length, poly->polynomial);

  if (new_line)
    fprintf(out_file, "\n");
}

/*
 * Initialize the algorithm with the default params.
 */
int initialize_rabin_polynomial_defaults()
{
  if (rabin_poly_init_completed != 0)
    return 1; //Nothing to do

  polynomial_lookup_buf = malloc(sizeof(uint64_t) * RAB_POLYNOMIAL_MAX_WIN_SIZE);

  if (polynomial_lookup_buf == NULL) {
    fprintf(stderr, "Could not initialize rabin polynomial lookaside buffer, out of memory\n");
    return 0;
  }

  int index = 0;
  uint64_t curPower = 1;
  //Initialize the lookup values we will need later
  for (index = 0; index < RAB_POLYNOMIAL_MAX_WIN_SIZE; index++) {
    //TODO check if max window size is a power of 2
    //and if so use shifters instead of multiplication
    polynomial_lookup_buf[index] = curPower;
    curPower *= rabin_polynomial_prime;
  }


  rabin_poly_init_completed = 1;

  return 1;
}


/**
 * Modifies the average block size, checking to make sure it doesn't
 * go above the max or below the min
 */
void change_average_rabin_block_size(int increment_mode)
{
  if (increment_mode && rabin_polynomial_average_block_size < rabin_polynomial_max_block_size) {
    rabin_polynomial_average_block_size++;
  } else if (!increment_mode && rabin_polynomial_average_block_size > rabin_polynomial_min_block_size) {
    rabin_polynomial_average_block_size--;
  }
}

/**
 * Initalizes the algorithm with the provided paramters
 */
static int initialize_rabin_polynomial(uint64_t prime, unsigned max_size, unsigned int min_size, unsigned int average_block_size)
{
  rabin_polynomial_prime = prime;
  rabin_polynomial_max_block_size = max_size;
  rabin_polynomial_min_block_size = min_size;
  rabin_polynomial_average_block_size = average_block_size;

  return initialize_rabin_polynomial_defaults();
}


/*
 * Generate a new fingerprint with the given info and add it to the tail
 */
struct rabin_polynomial *gen_new_polynomial(struct rabin_polynomial *tail, uint64_t total_len, uint16_t length, uint64_t rab_sum) {

  struct rabin_polynomial *next = malloc(sizeof(struct rabin_polynomial));

  if (next == NULL) {
    fprintf(stderr, "Could not allocate memory for rabin fingerprint record!");
    return NULL;
  }

  if (tail != NULL)
    tail->next_polynomial = next;

  next->next_polynomial = NULL;
  next->start = total_len - length;
  next->length = length;
  next->polynomial = rab_sum;

  return next;
}

/*
 * Writes out the fingerprint list in binary form
 */
int write_rabin_fingerprints_to_binary_file(FILE *file, struct rabin_polynomial *head)
{
  struct rabin_polynomial *poly = head;

  while (poly != NULL) {
    size_t ret_val = fwrite(poly, sizeof(struct rabin_polynomial), 1, file);

    if (ret_val == 0) {
      fprintf(stderr, "Could not write rabin polynomials to file.");
      return -1;
    }

    poly = poly->next_polynomial;
  }
  return 0;
}

/**
 * Reads a list of rabin fingerprints in binary form
 */
struct rabin_polynomial *read_rabin_polys_from_file_binary(FILE *file) {
  struct rabin_polynomial *head = gen_new_polynomial(NULL, 0, 0, 0);
  struct rabin_polynomial *tail = head;

  if (head == NULL)
    return NULL;

  size_t polys_read = fread(head, sizeof(struct rabin_polynomial), 1, file);

  while (polys_read != 0 && tail != NULL) {
    struct rabin_polynomial *cur_poly = gen_new_polynomial(tail, 0, 0, 0);
    fread(cur_poly, sizeof(struct rabin_polynomial), 1, file);
    tail = cur_poly;
  }

  return head;
}


/*
 * Deallocates the entire fingerprint list
 */
void free_rabin_fingerprint_list(struct rabin_polynomial *head)
{
  struct rabin_polynomial *cur_poly, *next_poly;

  cur_poly = head;

  while (cur_poly != NULL) {
    next_poly = cur_poly->next_polynomial;
    free(cur_poly);
    cur_poly = next_poly;
  }

}

/*
 * Gets the list of fingerprints from the given file
 */

struct rabin_polynomial *get_file_rabin_polys(FILE *file_to_read, const char* target, struct table_of_chunks* table) {
  initialize_rabin_polynomial_defaults();

  struct rab_block_info *block = NULL;
  char *file_data = malloc(RAB_FILE_READ_BUF_SIZE);

  if (file_data == NULL) {
    fprintf(stderr, "Could not allocate buffer for reading input file to rabin polynomial.\n");
    return NULL;
  }

  ssize_t bytes_read = fread(file_data, 1, RAB_FILE_READ_BUF_SIZE, file_to_read);

  while (bytes_read != 0) {
    total_read_bytes += bytes_read;
    block = read_rabin_block(file_data, bytes_read, block, NULL, NULL, target, table);
    bytes_read = fread(file_data, 1, RAB_FILE_READ_BUF_SIZE, file_to_read);
  }

  /*
   *deal with the last chunk of file
   */

  if (block) {
    free(file_data);
    struct rabin_polynomial *head = block->head;
    free(block);
    return head;
  } else { // file has not content
    return NULL;
  }
}

/**
 * Allocates an empty block
 */
struct rab_block_info *init_empty_block() {
  initialize_rabin_polynomial_defaults();
  struct rab_block_info *block = malloc(sizeof(struct rab_block_info));
  if (block == NULL) {
    fprintf(stderr, "Could not allocate rabin polynomial block, no memory left!\n");
    return NULL;
  }

  block->head = gen_new_polynomial(NULL, 0, 0, 0);

  if (block->head == NULL)
    return NULL; //Couldn't allocate memory

  block->tail = block->head;
  block->cur_roll_checksum = 0;
  block->total_bytes_read = 0;
  block->window_pos = 0;
  block->current_poly_finished = 0;

  block->current_window_data = malloc(sizeof(char)*rabin_sliding_window_size);

  if (block->current_window_data == NULL) {
    fprintf(stderr, "Could not allocate buffer for sliding window data!\n");
    free(block);
    return NULL;
  }
  int i;
  for (i = 0; i < rabin_sliding_window_size; i++) {
    block->current_window_data[i] = 0;
  }

  return block;
}

/**
 * Reads a block of memory and generates a rabin fingerprint list from it.
 * Since most of the time we will not end on a border, the function returns
 * a block struct, which keeps track of the current blocksum and rolling checksum
 */
struct rab_block_info *read_rabin_block(void *buf, ssize_t size, struct rab_block_info *cur_block, block_reached_func callback, void* user, const char* target, struct table_of_chunks* table) {
  struct rab_block_info *block;

  if (cur_block == NULL) {
    block = init_empty_block();
    if (block == NULL)
      return NULL;
  } else {
    block = cur_block;
  }

  ssize_t i;
  for (i = 0; i < size; i++) {
    char cur_byte = *((char *)buf + i);
    char pushed_out = block->current_window_data[block->window_pos];
    block->current_window_data[block->window_pos] = cur_byte;
    block->cur_roll_checksum = (block->cur_roll_checksum * rabin_polynomial_prime) + cur_byte;
    block->tail->polynomial = (block->tail->polynomial * rabin_polynomial_prime) + cur_byte;
    block->cur_roll_checksum -= (pushed_out * polynomial_lookup_buf[rabin_sliding_window_size]);
    
	//calculate features with cur_roll_checksum
    uint64_t cur_feature = block->cur_roll_checksum ;
    calculate_features(cur_feature, features, hash_of_fp);
    
    block->window_pos++;
    block->total_bytes_read++;
    block->tail->length++;

    if (block->window_pos == rabin_sliding_window_size) //Loop back around
      block->window_pos = 0;

    //If we hit our special value or reached the max win size create a new block
    if ((block->tail->length >= rabin_polynomial_min_block_size && (block->cur_roll_checksum % rabin_polynomial_average_block_size) == rabin_polynomial_prime) || block->tail->length == rabin_polynomial_max_block_size) {
      block->tail->start = block->total_bytes_read - block->tail->length;
 
      calculate_super_features(super_features,features);
      char* buffer = (char*)malloc(sizeof(char) * (block->tail->length));
      memcpy(buffer, (char*)(buf + i), block->tail->length);
        
      uint64_t fp = get_fp_of_block(buffer, block->tail->length);
 
	  struct chunk* cur_chunk = NULL;
	  cur_chunk = insert_fingerprint_in_table(table, fp, super_features, block->tail->length);
	  if(cur_chunk != NULL)
      {
        struct chunk* similar_chunk = NULL; 
		similar_chunk = find_similar_chunk(table, fp, super_features);
        if(similar_chunk != NULL)
        {  
            /*
             *find similar chunk, get its data of chunk, and compress it
             */
            char *ref = get_chunk_from_file(ref, similar_chunk, target);         
			unsigned char *delta = NULL;
            uint64_t len_of_delta = 0;
            zd_compress(ref, similar_chunk->length, buffer, block->tail->length, delta, &len_of_delta); 
            total_delta_bytes += len_of_delta;
            total_compress_bytes += block->tail->length - len_of_delta;
            cur_chunk->length = len_of_delta;
            /*
             *write difference to file
             */
            write_chunk_to_file(delta, len_of_delta, cur_chunk, target);
            if(delta != NULL)
                free(delta);
            free(ref);
        }
        else// not find similar chunk and insert it directly
        {
            write_chunk_to_file(buffer, block->tail->length, cur_chunk, target);
        }
      }
      else
      {
         total_duplicate_bytes += block->tail->length;
      }
      struct rabin_polynomial *new_poly = gen_new_polynomial(NULL, 0, 0, 0);
      block->tail->next_polynomial = new_poly;
      block->tail = new_poly;
      /*
       *test code
       */
      printf("total_read_bytes: %lu\n", total_read_bytes);
      printf("total_compress_bytes: %lu\n", total_compress_bytes);
      printf("total_delta_bytes: %lu\n", total_delta_bytes);
      printf("total_duplicate_bytes: %lu\n\n", total_duplicate_bytes);
      
      free(buffer);
      memset(super_features, 0, sizeof(super_features));
      memset(features, 0, sizeof(features));
      memset(hash_of_fp, 0, sizeof(hash_of_fp));

      if (i == size - 1)
        block->current_poly_finished = 1;
    }
  }
  block->tail->start = block->total_bytes_read - block->tail->length;

  if (block->current_poly_finished) {
      struct rabin_polynomial *new_poly = gen_new_polynomial(NULL, 0, 0, 0);
      block->tail->next_polynomial = new_poly;
      block->tail = new_poly;
      block->current_poly_finished = 0;
  }
 
  return block;
}

	

