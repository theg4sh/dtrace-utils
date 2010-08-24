/*
 * FILE:	dtrace_hash.c
 * DESCRIPTION:	Dynamic Tracing: probe hashing functions
 *
 * Copyright (C) 2010 Oracle Corporation
 */

#include <linux/slab.h>

#include "dtrace.h"

#define DTRACE_HASHSTR(hash, probe)	\
	dtrace_hash_str(*((char **)((uintptr_t)(probe) + (hash)->dth_stroffs)))
#define DTRACE_HASHEQ(hash, lhs, rhs)	\
	(strcmp(*((char **)((uintptr_t)(lhs) + (hash)->dth_stroffs)), \
		*((char **)((uintptr_t)(rhs) + (hash)->dth_stroffs))) == 0)

static uint_t dtrace_hash_str(char *p)
{
	uint_t	g;
	uint_t	hval = 0;

	while (*p) {
		hval = (hval << 4) + *p++;
		if ((g = (hval & 0xf0000000)) != 0)
			hval ^= g >> 24;

		hval &= ~g;
	}

	return hval;
}

dtrace_hash_t *dtrace_hash_create(uintptr_t stroffs, uintptr_t nextoffs,
				  uintptr_t prevoffs)
{
	dtrace_hash_t	*hash = kzalloc(sizeof (dtrace_hash_t), GFP_KERNEL);

	hash->dth_stroffs = stroffs;
	hash->dth_nextoffs = nextoffs;
	hash->dth_prevoffs = prevoffs;

	hash->dth_size = 1;
	hash->dth_mask = hash->dth_size - 1;

	hash->dth_tab = kzalloc(hash->dth_size *
				sizeof (dtrace_hashbucket_t *), GFP_KERNEL);

	return hash;
}

static void dtrace_hash_resize(dtrace_hash_t *hash)
{
	int			size = hash->dth_size, i, ndx;
	int			new_size = hash->dth_size << 1;
	int			new_mask = new_size - 1;
	dtrace_hashbucket_t	**new_tab, *bucket, *next;

	BUG_ON((new_size & new_mask) != 0);

	new_tab = kzalloc(new_size * sizeof (void *), GFP_KERNEL);

	for (i = 0; i < size; i++) {
		for (bucket = hash->dth_tab[i]; bucket != NULL;
		     bucket = next) {
			dtrace_probe_t *probe = bucket->dthb_chain;

			BUG_ON(probe == NULL);
			ndx = DTRACE_HASHSTR(hash, probe) & new_mask;

			next = bucket->dthb_next;
			bucket->dthb_next = new_tab[ndx];
			new_tab[ndx] = bucket;
		}
	}

	kfree(hash->dth_tab);
	hash->dth_tab = new_tab;
	hash->dth_size = new_size;
	hash->dth_mask = new_mask;
}

void dtrace_hash_add(dtrace_hash_t *hash, dtrace_probe_t *new)
{
	int			hashval = DTRACE_HASHSTR(hash, new);
	int			ndx = hashval & hash->dth_mask;
	dtrace_hashbucket_t	*bucket = hash->dth_tab[ndx];
	dtrace_probe_t		**nextp, **prevp;

	for (; bucket != NULL; bucket = bucket->dthb_next) {
		if (DTRACE_HASHEQ(hash, bucket->dthb_chain, new))
			goto add;
	}

	if ((hash->dth_nbuckets >> 1) > hash->dth_size) {
		dtrace_hash_resize(hash);
		dtrace_hash_add(hash, new);
		return;
	}

	bucket = kzalloc(sizeof (dtrace_hashbucket_t), GFP_KERNEL);
	bucket->dthb_next = hash->dth_tab[ndx];
	hash->dth_tab[ndx] = bucket;
	hash->dth_nbuckets++;

add:
	nextp = DTRACE_HASHNEXT(hash, new);

	BUG_ON(*nextp != NULL || *(DTRACE_HASHPREV(hash, new)) != NULL);

	*nextp = bucket->dthb_chain;

	if (bucket->dthb_chain != NULL) {
		prevp = DTRACE_HASHPREV(hash, bucket->dthb_chain);

		BUG_ON(*prevp != NULL);

		*prevp = new;
	}

	bucket->dthb_chain = new;
	bucket->dthb_len++;
}

dtrace_probe_t *dtrace_hash_lookup(dtrace_hash_t *hash,
				   dtrace_probe_t *template)
{
	int			hashval = DTRACE_HASHSTR(hash, template);
	int			ndx = hashval & hash->dth_mask;
	dtrace_hashbucket_t	*bucket = hash->dth_tab[ndx];

	for (; bucket != NULL; bucket = bucket->dthb_next) {
		if (DTRACE_HASHEQ(hash, bucket->dthb_chain, template))
			return bucket->dthb_chain;
	}

	return NULL;
}

int dtrace_hash_collisions(dtrace_hash_t *hash, dtrace_probe_t *template)
{
	int			hashval = DTRACE_HASHSTR(hash, template);
	int			ndx = hashval & hash->dth_mask;
	dtrace_hashbucket_t	*bucket = hash->dth_tab[ndx];

	for (; bucket != NULL; bucket = bucket->dthb_next) {
		if (DTRACE_HASHEQ(hash, bucket->dthb_chain, template))
			return bucket->dthb_len;
	}

	return 0;
}

void dtrace_hash_remove(dtrace_hash_t *hash, dtrace_probe_t *probe)
{
	int			ndx = DTRACE_HASHSTR(hash, probe) &
				      hash->dth_mask;
	dtrace_hashbucket_t	*bucket = hash->dth_tab[ndx];
	dtrace_probe_t		**prevp = DTRACE_HASHPREV(hash, probe);
	dtrace_probe_t		**nextp = DTRACE_HASHNEXT(hash, probe);

	for (; bucket != NULL; bucket = bucket->dthb_next) {
		if (DTRACE_HASHEQ(hash, bucket->dthb_chain, probe))
			break;
	}

	BUG_ON(bucket == NULL);

	if (*prevp == NULL) {
		if (*nextp == NULL) {
			/*
			 * This is the last probe in the bucket; we can remove
			 * the bucket.
			 */
			dtrace_hashbucket_t	*b = hash->dth_tab[ndx];

			BUG_ON(bucket->dthb_chain != probe);
			BUG_ON(b == NULL);

			if (b == bucket)
				hash->dth_tab[ndx] = bucket->dthb_next;
			else {
				while (b->dthb_next != bucket)
					b = b->dthb_next;

				b->dthb_next = bucket->dthb_next;
			}

			BUG_ON(hash->dth_nbuckets <= 0);

			hash->dth_nbuckets--;
			kfree(bucket);

			return;
		}

		bucket->dthb_chain = *nextp;
	} else
		*(DTRACE_HASHNEXT(hash, *prevp)) = *nextp;

	if (*nextp != NULL)
		*(DTRACE_HASHPREV(hash, *nextp)) = *prevp;
}
