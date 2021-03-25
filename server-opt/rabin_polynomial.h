#include <stdint.h>
#include <stdio.h>

#ifdef __APPLE__
#include <unistd.h>
#endif

#ifndef JOEL_TUCCI_RABIN_POLY_C
#define JOEL_TUCCI_RABIN_POLY_C

extern uint64_t rabin_polynomial_prime;
extern unsigned int rabin_sliding_window_size;

extern unsigned int rabin_polynomial_max_block_size;
extern unsigned int rabin_polynomial_min_block_size;

extern unsigned int rabin_polynomial_average_block_size;


/**
 * All the info needed for a rabin polynomial list, namely the start position in the file,
 * the length of the block, the checksum, and the next polynomial
 */
struct rabin_polynomial {
  uint64_t start;
  uint32_t length;
  uint64_t polynomial;
  struct rabin_polynomial *next_polynomial;
};

/*
 * Struct used to keep track of rabin polynomials for blocks of memory,
 * since the blocks may or may not end on a boundary, we have to save the
 * current rolling checksum, length, and block checksum so that we can
 * pick up were we left off
 */
struct rab_block_info {
  struct rabin_polynomial *head;
  struct rabin_polynomial *tail;
  uint64_t total_bytes_read;
  unsigned int window_pos;
  char current_poly_finished;
  char *current_window_data;
  uint64_t cur_roll_checksum;
  uint64_t current_block_checksum;
  uint64_t curr_roll_offset;
};

typedef void (*block_reached_func)(struct rabin_polynomial* result, void* user);

int initialize_rabin_polynomial_defaults();
struct rab_block_info *read_rabin_block(void *buf, ssize_t size, struct rab_block_info *cur_block, block_reached_func callback, void* user);

void change_average_rabin_block_size(int increment_mode);
int write_rabin_fingerprints_to_binary_file(FILE *file, struct rabin_polynomial *head);
struct rabin_polynomial *read_rabin_polys_from_file_binary(FILE *file);
void free_rabin_fingerprint_list(struct rabin_polynomial *head);

struct rabin_polynomial *gen_new_polynomial(struct rabin_polynomial *tail, uint64_t total_len, uint16_t length, uint64_t rab_sum);

void print_rabin_poly_to_file(FILE *out_file, struct rabin_polynomial *poly, int new_line);
void print_rabin_poly_list_to_file(FILE *out_file, struct rabin_polynomial *poly);
struct rabin_polynomial *get_file_rabin_polys(FILE *file_to_read);

#endif

