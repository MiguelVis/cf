/*	cf.h

	Management library for configuration files.

	Copyright (c) 2016 Miguel I. Garcia Lopez / FloppySoftware, Spain

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

	Revisions:

	06 Jul 2016 : Work begins.
	07 Jul 2016 : Minor changes and optimizations.
	08 Jul 2016 : Set max. # of key / value pairs on creation time.
	15 Jul 2016 : Added supported #defines.
	              Added '.' and '-' as valids character for key names.
	              Added ';' as valid character for comments.
	15 Jul 2016 : Ported to Pelles C from MESCC (under CP/M).
	16 Jul 2016 : Added CfGetAll().
	21 Oct 2016 : Solved a couple of bugs in CfRead() and xCfAdd(). Refactorized function names.
	24 Oct 2016 : Modified CfRead(), CfWrite(), CfSetKey() to support reading and writing empty lines and comments.
	01 Jan 2017 : Don't destroy CF on error in CfRead().
	16 Mar 2017 : Removed duplicated code in xCfAdd().

	Notes:

	Valid characters for key names: A..Z a..z 0..9 . _ -
	Valid characters for comments:  # ;
*/

#ifndef CF_H

#define CF_H

/* Dependencies
   ------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Public defines
   --------------
*/
#define CF struct xcf_entry

/* Public declarations
   -------------------
*/
CF *CfCreate(int max);
CF *CfDestroy(CF *cf);
int CfSetKey(CF *cf, char *key, char *value);
char *CfGetKey(CF *cf, char *key);
int CfRead(CF *cf, char *fname, int cmt);
int CfWrite(CF *cf, char *fname);
void CfGetAll(CF *cf, int (*funct)(char *key, char *value));
int CfGetBool(CF *cf, char *key, int def);
int CfGetInt(CF *cf, char *key, int def);
unsigned int CfGetUint(CF *cf, char *key, unsigned int def);
char *CfGetStr(CF *cf, char *key, char *def);
int CfSetBool(CF *cf, char *key, int value);
int CfSetStr(CF *cf, char *key, char *value);
void CfPrKeys(CF *cf);

/* Private defines
   ---------------
*/
#define XCF_DEBUG     1   // Enable or disable debug

#define XCF_BF_SIZE 130  // Size of buffer for file input: 128 + \n + 0

/* Private declarations
   --------------------
*/
struct xcf_entry {
	char **keys;
	char **values;
	int max;
};

char **xCfAlloc(int max);
void xCfFree(char **arr, int size);
int xCfFind(char **arr, int size, char *key);
int xCfAdd(char **arr, int size, char *data);
char *xCfSet(char **arr, char *data, int entry);
char *xCfDel(char **arr, int entry);
char *xCfLfSpaces(char *s);
char *xCfRtSpaces(char *s);

/* Private globals
   ---------------
*/
char xcf_buf[XCF_BF_SIZE];

// --------------------
// -- CORE FUNCTIONS --
// --------------------

/* Create a configuration buffer
   -----------------------------
   Return address, or NULL on failure.
*/
CF *CfCreate(int max)
{
	CF *cf;

	// Alloc memory
	if((cf = malloc(sizeof(CF)))) {
		if((cf->keys = xCfAlloc(max))) {
			if((cf->values = xCfAlloc(max))) {

				// Store the rest of fields
				cf->max = max;
			
				// Success
				return cf;
			}

			// Failure
			xCfFree(cf->keys, max);
		}

		// Failure
		free(cf);
	}

	// Failure
	return NULL;
}

/* Destroy a configuration buffer
   ------------------------------
   Return NULL.
*/
CF *CfDestroy(CF *cf)
{
	// Free the arrays and its entries
	xCfFree(cf->keys, cf->max);
	xCfFree(cf->values, cf->max);

	// Free the buffer
	free(cf);

	// Done
	return NULL;
}

/* Set the value of a key
   ----------------------
   Return 0 on success, or -1 on failure.
*/
int CfSetKey(CF *cf, char *key, char *value)
{
	int entry;

	// Get entry number
	if((entry = (*key && *key != '#' && *key != ';') ? xCfFind(cf->keys, cf->max, key) : -1) != -1) {

		// The key exists: change its value
		if(xCfSet(cf->values, value, entry)) {

			// Success
			return 0;
		}
	}
	else {
		// The key doesn't exists: add it
		if((entry = xCfAdd(cf->keys, cf->max, key)) != -1) {

			// Add the value
			if(xCfSet(cf->values, value, entry)) {

				// Success
				return 0;
			}

			// Failure
			xCfDel(cf->keys, entry);
		}

		// Failure
	}

	// Failure
	return -1;
}

/* Get the raw value of a key
   --------------------------
   Return pointer to the value, or NULL on failure.
*/
char *CfGetKey(CF *cf, char *key)
{
	int entry;
	char **arr;

	// Get entry number
	if((entry = xCfFind(cf->keys, cf->max, key)) != -1) {

		// Get array address
		arr = cf->values;

		// Success
		return arr[entry];
	}

	// Failure
	return NULL;
}

// ------------------------
// -- FILE I/O FUNCTIONS --
// ------------------------

/* Read a configuration buffer from a file
   ---------------------------------------
   Return 0 on success, or -1 on failure.
*/
int CfRead(CF *cf, char *fname, int cmt)
{
	FILE *fp;
	int err, k;
	char *bf, *key, cmt_key[2];

	// Default: no errors
	err = cmt_key[1] = 0;

	// Open file
	if((fp = fopen(fname, "r"))) {
		while(fgets(xcf_buf, XCF_BF_SIZE, fp)) {

			// Get the length of the string
			k = strlen(xcf_buf);

			// Remove the trailing new line if any,
			// and check for too long lines.
			if(xcf_buf[k - 1] == '\n')
				xcf_buf[k - 1] = '\0';
			else if(k == XCF_BF_SIZE - 1) {
				// Line too long
				err = -1; break;
			}

			// Remove spaces on the left
			bf = xCfLfSpaces(xcf_buf);

			// Remove spaces on the right
			bf = xCfRtSpaces(bf);

			// Skip comments and empty lines
			if(!(*bf) || *bf == '#' || *bf == ';') {
				if(cmt)	{
					cmt_key[0] = *bf;

					if(*bf) {
						// Remove spaces on the left
						bf = xCfLfSpaces(bf + 1);
					}

					if(CfSetKey(cf, cmt_key, bf)) {
						//CfDestroy(cf);
						err = -1;
						break;
					}
				}

				continue;
			}

			// Set the pointer to the key name
			key = bf;

			// Go upto the end of the name
			while(isalnum(*bf) || *bf == '.' || *bf == '_' || *bf == '-')
				++bf;

			// Check if it's a legal name
			// and the next character is valid.
			if(key == bf || (*bf != ' ' && *bf != '\t' && *bf != '=')) {
				err = -1; break;
			}

			// Get the next character
			k = *bf;

			// Set the end of the name, and skip the next character
			*bf++ = '\0';

			// Check for =
			if(k != '=') {
				// Skip spaces
				bf = xCfLfSpaces(bf);

				// Check for =
				if(*bf != '=') {
					err = -1; break;
				}

				// Skip =
				++bf;
			}

			// Skip spaces
			bf = xCfLfSpaces(bf);

			// Check if there is a value
			if(!(*bf)) {
				err = -1; break;
			}

			// Add the key / value pair to the configuration buffer
			if(CfSetKey(cf, key, bf)) {
				//CfDestroy(cf);
				err = -1;
				break;
			}
		}

		// Close file
		fclose(fp);

		// Success or failure
		return err;
	}

	// Failure
	return -1;
}

/* Write a configuration buffer to a file
   --------------------------------------
   Return 0 on success, or -1 on failure.
*/
int CfWrite(CF *cf, char *fname)
{
	FILE *fp;
	char **arrk, **arrv;
	int max, i;

	// Open file
	if((fp = fopen(fname, "w"))) {

		// Get fields
		arrk = cf->keys;
		arrv = cf->values;
		max  = cf->max;

		// Write key / value pairs
		for(i = 0; i < max; ++i) {
			if(arrk[i]) {
				if(*arrk[i]) {
					fprintf(fp, (*arrk[i] != '#' && *arrk[i] != ';') ? "%s = %s\n" : "%s %s\n", arrk[i], arrv[i]);
				}
				else {
					fputc('\n', fp);
				}
			}
		}

		// Success or failure
		return (fclose(fp) ? -1 : 0);
	}

	// Failure
	return -1;
}

// -------------------
// -- GET FUNCTIONS --
// -------------------

/* Get all keys
  -------------
*/
void CfGetAll(CF *cf, int (*funct)(char *key, char *value))
{
	char **arrk, **arrv;
	int max, i;

	// Get fields
	arrk = cf->keys;
	arrv = cf->values;
	max  = cf->max;

	// Iterate the array
	for(i = 0; i < max; ++i) {
		if(arrk[i]) {
			if(funct(arrk[i], arrv[i]))
				break;
		}
	}
}

/* Get the true / false value of a key
   -----------------------------------
   Return 1 for true, 0 for false, or the default value on failure.
*/
int CfGetBool(CF *cf, char *key, int def)
{
	char *value;

	// Get value
	if((value = CfGetKey(cf, key))) {

		// Check for true or false
		if(!strcmp(value, "true"))
			return 1;
		else if(!strcmp(value, "false"))
			return 0;

		// Failure
	}

	// Failure
	return def;
}

/* Get the int value of a key
   --------------------------
   Return an int, or the default value on failure.
*/
int CfGetInt(CF *cf, char *key, int def)
{
	char *value;

	// Get value
	if((value = CfGetKey(cf, key))) {

		// Return the int value
		return atoi(value);
	}

	// Failure
	return def;
}

/* Get the unsigned int value of a key
   -----------------------------------
   Return an unsigned int, or the default value on failure.
*/
unsigned int CfGetUint(CF *cf, char *key, unsigned int def)
{
	char *value;
	unsigned int val;

	// Setup value
	val = 0;

	// Get value
	if((value = CfGetKey(cf, key))) {

		// Compute the value
		while(isdigit(*value))
			val = val * 10 + (*value++ - '0');

		// Check end of value
		if(!(*value))
			return val;

		// Failure
	}

	// Failure
	return def;
}

/* Get the string value of a key
   -----------------------------
   Return a string, or the default value on failure.
*/
char *CfGetStr(CF *cf, char *key, char *def)
{
	char *value;
	int k;

	// Get value
	if((value = CfGetKey(cf, key))) {

		// Check for quoted strings
		if(*value == '\"') {
			// Get the string length
			k = strlen(value);

			// Check for trailing quote
			if(k >= 2 && value[k - 1] == '\"') {
				// Remove trailing quote
				value[k - 1] = '\0';

				// Skip quote on the left
				++value;
			}
			else {
				// Failure
				return def;
			}
		}

		// Return the value
		return value;
	}

	// Failure
	return def;
}

// -------------------
// -- SET FUNCTIONS --
// -------------------

/* Set the true / false value of a key
   -----------------------------------
   Return 0 on success, or -1 on failure.
*/
int CfSetBool(CF *cf, char *key, int value)
{
	// Set the value
	// and return the result.
	return CfSetKey(cf, key, value ? "true" : "false");
}

/* Set the string value of a key
   -----------------------------
   Return 0 on success, or -1 on failure.
*/
int CfSetStr(CF *cf, char *key, char *value)
{
	char *tmp;
	int size, result;

	// Compute size
	size = strlen(value) + 2;

	// Alloc memory
	if((tmp = malloc(size))) {
		// Setup string
		*tmp = '\"';
		strcpy(tmp + 1, value);
		tmp[size - 2] = '\"';

		// Set the key
		result = CfSetKey(cf, key, tmp);

		// Free allocated memory
		free(tmp);

		// Return success or failure
		return result;
	}

	// Failure
	return -1;
}

// ---------------------
// -- DEBUG FUNCTIONS --
// ---------------------

#if XCF_DEBUG

/* Print keys and values
   ---------------------
*/
void CfPrKeys(CF *cf)
{
	char **arrk, **arrv;
	int i;

	arrk = cf->keys;
	arrv = cf->values;

	for(i = 0; i < cf->max; ++i) {
		if(arrk[i]) {
			if(*arrk[i]) {
				printf((*arrk[i] == '#' || *arrk[i] == ';') ? "%02d : %s %s\n" : "%02d : %s = %s\n", i, arrk[i], arrv[i]);
			}
			else {
				printf("%02d :\n", i);
			}
		}
	}
}

#endif

// -------------------------------
// -- PRIVATE FUNCTIONS: ARRAYS --
// -------------------------------

/* Allocate and clear an array
   ---------------------------
   Return array address, or NULL on failure.
*/
char **xCfAlloc(int max)
{
	char **arr;
	int i;

	// Alloc memory for the array
	if((arr = malloc(sizeof(char **) * max))) {

		// Clear the array
		for(i = 0; i < max; ++i)
			arr[i] = NULL;

		// Success
		return arr;
	}

	// Failure
	return NULL;
}

/* Free an array and its entries
   -----------------------------
*/
void xCfFree(char **arr, int size)
{
	int i;

	// Free allocated entries
	for(i = 0; i < size; ++i) {
		if(arr[i])
			free(arr[i]);
	}

	// Free the array
	free(arr);
}

/* Find a key
   ----------
   Return entry number, or -1 on failure.
*/
int xCfFind(char **arr, int size, char *key)
{
	int i;

	// Search for data in the array
	for(i = 0; i < size; ++i) {
		if(arr[i]) {
			if(!strcmp(arr[i], key)) {

				// Found
				return i;
			}
		}
	}

	// Not found
	return -1;
}

/* Add a value to an array
   -----------------------
   Return entry number, or -1 on failure.
*/
int xCfAdd(char **arr, int size, char *data)
{
	int i;

	// Search for unallocated entry in the array
	for(i = 0; i < size; ++i) {
		if(!arr[i]) {

			// Set the value and return success or failure
			return xCfSet(arr, data, i) ? i : -1;
		}
	}

	// Failure
	return -1;
}

/* Set a value in an array
   -----------------------
   Return pointer to the entry data, or NULL on failure.
*/
char *xCfSet(char **arr, char *data, int entry)
{
	char *str;

	// Allocate memory for data
	if((str = malloc(strlen(data) + 1))) {

		// Free old data if needed
		if(arr[entry])
			free(arr[entry]);

		// Copy the data, update the pointer,
		// and return pointer to the entry data.
		return (arr[entry] = strcpy(str, data));
	}

	// Failure
	return NULL;
}

/* Delete a value in an array
   --------------------------
   Return NULL.
*/
char *xCfDel(char **arr, int entry)
{
	// Free old data if needed
	if(arr[entry])
		free(arr[entry]);

	// Update pointer and return NULL
	return (arr[entry] = NULL);
}

// --------------------------------
// -- PRIVATE FUNCTIONS: STRINGS --
// --------------------------------

/* Skip spaces on the left of a string
   -----------------------------------
   Return pointer to the first non space character.
*/
char *xCfLfSpaces(char *s)
{
	// Skip spaces
	while(*s == ' ' || *s == '\t')
		++s;

	// Return pointer
	return s;
}

char *xCfRtSpaces(char *s)
{
	char *rtp;

	// Set pointer to the last character on the right
	rtp = s + strlen(s) - 1;

	// Search for the last non space character
	while(rtp >= s && (*rtp == ' ' || *rtp == '\t'))
		rtp--;

	// Set trailing zero
	*(++rtp) = '\0';

	// Return pointer to string
	return s;
}

#endif

