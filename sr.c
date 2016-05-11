
/**
 * @file sr.c
 *
 * @brief sequence reader impl.
 */
#include <stdint.h>
#include <stdlib.h>
#include "sr.h"


/* inline directive */
#define _force_inline				inline

/**
 * @struct sr_s
 */
struct sr_s {
	fna_t *fna;
	gref_acv_t *acv;
	gref_idx_t *idx;
	struct sr_params_s params;
};

/**
 * @fn sr_init
 */
sr_t *sr_init(
	char const *path,
	sr_params_t const *params)
{
	/* check arguments sanity */
	if(path == NULL) {
		goto _sr_init_error_handler;
	}

	/* malloc mem */
	struct sr_s *sr = (struct sr_s *)malloc(sizeof(struct sr_s));

	/* create fna object */
	sr->fna = fna_init(path, FNA_PARAMS(
		.file_format = params->file_format,
		.seq_encode = FNA_4BIT));
	if(sr->fna == NULL) {
		goto _sr_init_error_handler;
	}

	/* copy params */
	sr->params = *params;
	return((sr_t *)sr);

_sr_init_error_handler:;
	fna_close(sr->fna); sr->fna = NULL;
	free(sr);
	return(NULL);
}

/**
 * @fn sr_clean
 */
void sr_clean(
	sr_t *sr)
{
	if(sr == NULL) { return; }

	gref_clean(sr->idx); sr->idx = NULL;
	fna_close(sr->fna); sr->fna = NULL;
	free(sr);
	return;
}

/**
 * @fn sr_dump_seq
 */
static _force_inline
void sr_dump_seq(
	sr_t *sr)
{
	/* init pool object */
	gref_pool_t *pool = gref_init_pool(GREF_PARAMS(
		.k = sr->params.k,
		.seq_direction = sr->params.seq_direction,
		.seq_format = GREF_4BIT,
		.copy_mode = GREF_COPY,
		.num_threads = sr->params.num_threads));

	/* dump sequence */
	fna_seq_t *seq = NULL;
	while((seq = fna_read(sr->fna)) != NULL) {

		if(seq->type == FNA_SEGMENT) {
			gref_append_segment(pool,
				seq->s.segment.name,
				seq->s.segment.name_len,
				seq->s.segment.seq,
				seq->s.segment.seq_len);
		} else if(seq->type == FNA_LINK) {
			/* check cigar */
			if(seq->s.link.cigar_len != 0) {
				debug("overlapping link is not supported.");
				break;
			} else {
				gref_append_link(pool,
					seq->s.link.from,
					seq->s.link.from_len,
					seq->s.link.from_ori,
					seq->s.link.to,
					seq->s.link.to_len,
					seq->s.link.to_ori);
			}
		} else {
			/* unknown type */
			debug("unknown sequence type appeared.");
		}

		fna_seq_free(seq);
	}
	
	/* freeze */
	sr->acv = gref_freeze_pool(pool);

	/* close fna context */
	fna_close(sr->fna); sr->fna = NULL;
	return;
}

/**
 * @fn sr_get_index
 */
gref_idx_t *sr_get_index(
	sr_t *sr)
{
	/* check if index is already built */
	if(sr->idx != NULL) {
		return(sr->idx);
	}

	/* archive and build index */
	if(sr->acv == NULL) {
		sr_dump_seq(sr);
	}
	sr->idx = gref_build_index();

	if(sr->idx == NULL) {
		sr->acv = NULL;
		return(NULL);
	}
	return(sr->idx);
}

/**
 * @fn sr_get_iter_graph
 */
static _force_inline
gref_iter_t *sr_get_iter_graph(
	sr_t *sr)
{
	return(iter);
}

/**
 * @fn sr_get_iter_read
 */
static _force_inline
gref_iter_t *sr_get_iter_read(
	sr_t *sr)
{
	return(iter);
}

/**
 * @fn sr_get_iter
 */
gref_iter_t *sr_get_iter(
	sr_t *sr)
{
	/* check if archive is already built */
	if(sr->acv == NULL) {
		sr_dump_seq(sr);
	}
	return(NULL);
}

/**
 * @fn sr_get_path
 */
char const *sr_get_path(
	sr_t const *sr)
{
	return(sr->fna->path);
}

/**
 * end of sr.c
 */
