
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
	uint8_t format;				/* equal to fna_params_t.file_format */
	uint8_t reserved1;
	uint16_t num_threads;
	uint16_t reserved2;
	void *lmm;					/* lmm memory manager */
};
typedef struct sr_params_s sr_params_t;

#define SR_PARAMS(...)		( &((struct sr_params_s const){ __VA_ARGS__ }) )

/**
 * @struct sr_gref_s
 * @brief gref and iter container
 */
struct sr_gref_s {
	void *reserved1;
	char const *path;
	gref_t const *gref;
	gref_iter_t *iter;
	uint32_t reserved2[2];
};

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
struct sr_gref_s *sr_get_index(
	sr_t *sr);

/**
 * @fn sr_get_iter
 */
struct sr_gref_s *sr_get_iter(
	sr_t *sr);

/**
 * @fn sr_free
 */
void sr_free(
	struct sr_gref_s *gref);

/**
 * end of sr.h
 */
