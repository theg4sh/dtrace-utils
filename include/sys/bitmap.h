/*
 * Oracle Linux DTrace.
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at
 * http://oss.oracle.com/licenses/upl.
 */

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/


#ifndef _SYS_BITMAP_H
#define	_SYS_BITMAP_H

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Operations on bitmaps of arbitrary size
 * A bitmap is a vector of 1 or more ulong_t's.
 * The user of the package is responsible for range checks and keeping
 * track of sizes.
 */

#ifdef _LP64
#define	BT_ULSHIFT	6 /* log base 2 of BT_NBIPUL, to extract word index */
#define	BT_ULSHIFT32	5 /* log base 2 of BT_NBIPUL, to extract word index */
#else
#define	BT_ULSHIFT	5 /* log base 2 of BT_NBIPUL, to extract word index */
#endif

#define	BT_NBIPUL	(1 << BT_ULSHIFT)	/* n bits per ulong_t */
#define	BT_ULMASK	(BT_NBIPUL - 1)		/* to extract bit index */

#ifdef _LP64
#define	BT_NBIPUL32	(1 << BT_ULSHIFT32)	/* n bits per ulong_t */
#define	BT_ULMASK32	(BT_NBIPUL32 - 1)	/* to extract bit index */
#define	BT_ULMAXMASK	0xffffffffffffffff	/* used by bt_getlowbit */
#else
#define	BT_ULMAXMASK	0xffffffff
#endif

/*
 * bitmap is a ulong_t *, bitindex an index_t
 *
 * The macros BT_WIM and BT_BIW internal; there is no need
 * for users of this package to use them.
 */

/*
 * word in map
 */
#define	BT_WIM(bitmap, bitindex) \
	((bitmap)[(bitindex) >> BT_ULSHIFT])
/*
 * bit in word
 */
#define	BT_BIW(bitindex) \
	(1UL << ((bitindex) & BT_ULMASK))

#ifdef _LP64
#define	BT_WIM32(bitmap, bitindex) \
	((bitmap)[(bitindex) >> BT_ULSHIFT32])

#define	BT_BIW32(bitindex) \
	(1UL << ((bitindex) & BT_ULMASK32))
#endif

/*
 * These are public macros
 *
 * BT_BITOUL == n bits to n ulong_t's
 */
#define	BT_BITOUL(nbits) \
	(((nbits) + BT_NBIPUL - 1l) / BT_NBIPUL)
#define	BT_SIZEOFMAP(nbits) \
	(BT_BITOUL(nbits) * sizeof (ulong_t))
#define	BT_TEST(bitmap, bitindex) \
	((BT_WIM((bitmap), (bitindex)) & BT_BIW(bitindex)) ? 1 : 0)
#define	BT_SET(bitmap, bitindex) \
	{ BT_WIM((bitmap), (bitindex)) |= BT_BIW(bitindex); }
#define	BT_CLEAR(bitmap, bitindex) \
	{ BT_WIM((bitmap), (bitindex)) &= ~BT_BIW(bitindex); }

#ifdef _LP64
#define	BT_BITOUL32(nbits) \
	(((nbits) + BT_NBIPUL32 - 1l) / BT_NBIPUL32)
#define	BT_SIZEOFMAP32(nbits) \
	(BT_BITOUL32(nbits) * sizeof (uint_t))
#define	BT_TEST32(bitmap, bitindex) \
	((BT_WIM32((bitmap), (bitindex)) & BT_BIW32(bitindex)) ? 1 : 0)
#define	BT_SET32(bitmap, bitindex) \
	{ BT_WIM32((bitmap), (bitindex)) |= BT_BIW32(bitindex); }
#define	BT_CLEAR32(bitmap, bitindex) \
	{ BT_WIM32((bitmap), (bitindex)) &= ~BT_BIW32(bitindex); }
#endif /* _LP64 */


#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_BITMAP_H */
