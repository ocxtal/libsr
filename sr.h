
/**
 * @file sr.h
 *
 * @brief sequence reader implementation
 */
#ifndef _AW_H_INCLUDED
#define _AW_H_INCLUDED

#include <stdint.h>

/**
 * @type sr_t
 */
typedef struct sr_s sr_t;

/**
 * @struct sr_params_s
 */
struct sr_params_s {
	uint8_t k;					/* kmer length */
	uint8_t seq_direction;		/* FW_ONLY or FW_RV */
	uint8_t file_format;		/* equal to fna_params_t.file_format */
	uint8_t reserved;
	uint16_t num_threads;
	uint16_t reserved2;
};
typedef struct sr_params_s sr_params_t;

#define SR_PARAMS(...)		( &((struct sr_params_s const){ __VA_ARGS__ }) )

/**
 * @fn sr_init
 */
sr_t *sr_init(
	char const *path,
	sr_params_t const *params);

/**
 * @fn sr_clean
 */
void sr_clean(
	sr_t *sr);

/**
 * @fn sr_get_index
 */
gref_idx_t *sr_get_index(
	sr_t *sr);

/**
 * @fn sr_get_iter
 */
gref_iter_t *sr_get_iter(
	sr_t *sr);

/**
 * @fn sr_get_path
 */
char const *sr_get_path(
	sr_t *sr);

/**
 * end of sr.h
 */
