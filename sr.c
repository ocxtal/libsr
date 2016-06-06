
/**
 * @file sr.c
 *
 * @brief sequence reader impl.
 */
#include <stdint.h>
#include <stdlib.h>
#include "lmm.h"
#include "sr.h"


/* constants */
#define SR_SINGLE_READ_MEM_SIZE		( 4096 )

/* inline directive */
#define _force_inline				inline

/**
 * @struct sr_s
 */
struct sr_s {
	lmm_t *lmm;
	fna_t *fna;
	gref_acv_t *acv;
	gref_idx_t *idx;
	int64_t count;
	struct sr_gref_s (*iter_read)(
		sr_t *sr);
	struct sr_params_s params;
	struct gref_params_s gref_params;
};

/**
 * @struct sr_gref_intl_s
 * @brief gref and iter container
 */
struct sr_gref_intl_s {
	lmm_t *lmm;
	char const *path;
	gref_t *gref;
	gref_iter_t *iter;
	int32_t ref_count;
	uint32_t reserved2;
};

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
		.num_threads = sr->params.num_threads,
		.lmm = sr->lmm));

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
struct sr_gref_s *sr_get_index(
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
	sr->idx = gref_build_index(&sr->acv);

	if(sr->idx == NULL) {
		sr->acv = NULL;
		return(NULL);
	}

	return((struct sr_gref_s){
		.lmm = NULL,
		.path = sr->fna->path,
		.gref = (gref_t const *)sr->idx,
		.iter = NULL,
		.ref_count = 1
	});
}

/**
 * @fn sr_get_iter_graph
 */
static
struct sr_gref_s *sr_get_iter_graph(
	sr_t *sr)
{
	/* check if archive is already built */
	if(sr->acv == NULL) {
		sr_dump_seq(sr);
	}

	return((struct sr_gref_s){
		.lmm = NULL,
		.path = (gref_t const *)sr->acv,
		.iter = gref_iter_init(sr->acv),
		.ref_count = 1
	});
}

/**
 * @fn sr_get_iter_read
 */
static
struct sr_gref_s *sr_get_iter_read(
	sr_t *sr)
{
	if(sr->fna == NULL) {
		return(NULL);
	}

	/* gref objects */
	gref_acv_t *acv = NULL;

	/* init local memory */
	lmm_t *lmm_read = lmm_init(NULL, SR_SINGLE_READ_MEM_SIZE);

	/* read a sequence */
	fna_seq_t *seq = NULL;
	while((seq = fna_read(sr->fna)) != NULL) {
		if(seq->type == FNA_SEGMENT) {
			gref_pool_t *pool = gref_init_pool(&sr->gref_params);
			gref_append_segment(pool,
				seq->s.segment.name,
				seq->s.segment.name_len,
				seq->s.segment.seq,
				seq->s.segment.seq_len);
			acv = gref_freeze_pool(pool);
			iter = gref_iter_init(acv);
			break;
		}
	}

	if(seq == NULL) {
		fna_close(sr->fna); sr->fna = NULL;
		return(NULL);
	}

	return((struct sr_gref_s){
		.lmm = lmm_read,
		.path = (gref_t const *)acv,
		.iter = gref_iter_init(acv),
		.ref_count = 1
	});
}

/**
 * @fn sr_get_iter
 */
struct sr_gref_s *sr_get_iter(
	sr_t *sr)
{
	return(sr->iter_read(sr));
}

/**
 * @fn sr_free
 */
void sr_free(
	struct sr_gref_s *gref)
{
	return;
}

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

	struct sr_params_s const default_params = { 0 };
	params = (params == NULL) ? &default_params : params;

	/* malloc mem */
	lmm_t *lmm = (lmm_t *)params->lmm;
	struct sr_s *sr = (struct sr_s *)lmm_malloc(lmm, sizeof(struct sr_s));
	memset(sr, 0, sizeof(struct sr_s));
	sr->lmm = lmm;

	/* create fna object */
	sr->fna = fna_init(path, FNA_PARAMS(
		.file_format = params->file_format,
		.seq_encode = FNA_4BIT,
		.lmm = lmm));
	if(sr->fna == NULL) {
		goto _sr_init_error_handler;
	}

	/* copy params */
	sr->params = *params;
	sr->gref_params = (struct gref_params_s){
		.k = sr->params.k,
		.seq_direction = sr->params.seq_direction,
		.seq_format = GREF_4BIT,
		.copy_mode = GREF_NOCOPY,
		.num_threads = sr->params.num_threads,
		.lmm = NULL,
	};
	return((sr_t *)sr);

_sr_init_error_handler:;
	fna_close(sr->fna); sr->fna = NULL;
	lmm_free(lmm, sr);
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
 * end of sr.c
 */
