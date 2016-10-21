/*	cf.c

	Some tests for CF.

	Copyright (c) 2016 Miguel Garcia / FloppySoftware

	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License as published by the
	Free Software Foundation; either version 3, or (at your option) any
	later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

	Author's contact:

		www.floppysoftware.es
		cpm-connections.blogspot.com
		floppysoftware@gmail.com

	To compile with MESCC:

		cc cf
		ccopt cf
		zsm cf
		hextocom cf

	Usage:

		cf

	Revisions:

		06 Jul 2016 : Work begins.
		21 Oct 2016 : Refactorized CF function names.

		--

		15 Jul 2016 : ...

*/

/* Standard libraries
   ------------------
*/
#include <stdlib.h>
#include <stdio.h>


/* Project libraries
   -----------------
*/
#include "cf.h"

/* Globals
   -------
*/
CF *cf;

/* Private declarations
   --------------------
*/
void set_key(char *key, char *value);
void set_bool(char *key, int value);
void set_str(char *key, char *value);
void pr_keys(CF *cf);
int pr_one_key(char *key, char *value);
void error(char *msg);

/* Program entry
   -------------
*/
int main(int argc, char **argv)
{
	int k;

	printf("Creating CF\n\n");

	if(!(cf = CfCreate(6)))
		error("Can't create CF");

	set_key("title", "That's cool!");
	set_key("author", "Jim Brown");
	set_key("year", "1969");
	set_key("pages", "150");
	set_str("summary", "This book, blah, blah, blah...");
	set_bool("lent", 1);

	// This should cause an error
	set_key("publisher", "This should cause an error: no more entries");

	printf("\n");
	pr_keys(cf);

	set_key("year", "1977");

	printf("\n");
	pr_keys(cf);

	printf("Writting test.cf\n\n");

	if(CfWrite(cf, "test.cf"))
		error("Can't write test.cf");

	printf("Destroying CF\n\n");

	CfDestroy(cf);

	// -------------------------------------------

	printf("Creating CF\n\n");

	if(!(cf = CfCreate(8)))
		error("Can't create CF");

	printf("Reading test.cf into CF\n\n");

	if(CfRead(cf, "test.cf"))
		error("Can't read test.cf");

	pr_keys(cf);

	printf("Reading test_2.cf into CF\n\n");

	if((k = CfRead(cf, "test_2.cf"))) { printf("!!%d!!\n", k);
		error("Can't read test_2.cf");}

	pr_keys(cf);

	CfGetAll(cf, pr_one_key);
	printf("\n");

	printf("Title     >> %s\n", CfGetKey(cf, "title"));
	printf("Author    >> %s\n", CfGetStr(cf, "author", "unknown"));
	printf("Publisher >> %s\n", CfGetStr(cf, "publisher", "n/a"));
	printf("Year      >> %d\n", CfGetUint(cf, "year", 9999));
	printf("Pages     >> %d\n", CfGetInt(cf, "pages", 9999));
	printf("Summary   >> %s\n", CfGetStr(cf, "summary", "n/a"));
	printf("Lent      >> %s\n", CfGetBool(cf, "lent", 0) ? "Yes" : "No");
	printf("To        >> %s\n", CfGetKey(cf, "lent_to"));
	printf("Expires   >> %s\n", CfGetKey(cf, "lend_expires"));

	printf("\n");

	printf("Destroying CF\n\n");

	CfDestroy(cf);

	// -------------------------------------------

	printf("Done\n");
}

void set_key(char *key, char *value)
{
	int result;

	result = CfSetKey(cf, key, value);

	printf("Set %s = %s%s\n", key, value, result ? " --> ERROR" : "");
}

void set_bool(char *key, int value)
{
	int result;

	result = CfSetBool(cf, key, value);

	printf("Set %s = %s%s\n", key, value ? "true" : "false", result ? " --> ERROR" : "");
}

void set_str(char *key, char *value)
{
	int result;

	result = CfSetStr(cf, key, value);

	printf("Set %s = \"%s\"%s\n", key, value, result ? " --> ERROR" : "");
}

void pr_keys(CF *cf)
{
	CfPrKeys(cf);

	printf("\n");
}

int pr_one_key(char *key, char *value)
{
	printf("%s = %s\n", key, value);

	return 0;
}

void error(char *msg)
{
	printf("ERROR: %s\n", msg);

	exit(-1);
}


