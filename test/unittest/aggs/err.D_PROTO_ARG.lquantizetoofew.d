/*
 * Oracle Linux DTrace.
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at
 * http://oss.oracle.com/licenses/upl.
 */

/*
 * ASSERTION:
 *   lquantize() should not accept a non-scalar value
 *
 * SECTION: Aggregations/Aggregations
 *
 */

BEGIN
{
	@a[pid] = lquantize(probefunc, probefunc, probefunc, probefunc);
}

