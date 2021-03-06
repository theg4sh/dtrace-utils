/*
 * Oracle Linux DTrace.
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at
 * http://oss.oracle.com/licenses/upl.
 */

/*
 * ASSERTION:
 * Test the declaration of a forward tag using a translator definition.
 *
 * SECTION: Translators/Translator Declarations
 *
 *
 */

#pragma D option quiet

struct input_struct {
	int i;
	char c;
} *uvar;

struct output_struct {
	int myi;
	char myc;
};

translator struct output_struct < struct callmewhatever *idontcare >
{
	myi = ((struct input_struct *) uvar)->i;
	myc = ((struct input_struct *) uvar)->c;
};

BEGIN
{
	printf("This test defines a forward tag");
	exit(0);
}
