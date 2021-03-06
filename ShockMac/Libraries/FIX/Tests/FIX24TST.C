/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/
/*
 * $RCSfile: fix24tst.c $
 * $Author: rex $
 * $Date: 1993/11/11 13:51:20 $
 *
 * program to test fixed-point math functions.
 *
 * $Log: fix24tst.c $
 * Revision 1.3  1993/11/11  13:51:20  rex
 * Added atofix24() test
 * 
 * Revision 1.2  1993/02/15  12:15:36  dfan
 * more fix24 functions
 * 
 * Revision 1.1  1993/02/15  11:40:12  dfan
 * Initial revision
 * 
 * Revision 1.3  1993/01/28  12:29:03  dfan
 * sqrt test
 * 
 * Revision 1.2  1993/01/22  09:57:57  dfan
 * Tests the new functions now
 * 
 * Revision 1.1  1992/09/16  20:18:11  kaboom
 * Initial revision
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fix.h"

#define MAX_ARGS 8

#define sign(x) ((x)>0?1:-1)

typedef struct command {
	char *str;
	uchar cmd;
} command;

int num_args;
long args[MAX_ARGS];
bool args_neg[MAX_ARGS];
char cmd;

#define CMD_NONE			0
#define CMD_ADD			1
#define CMD_SUB			2
#define CMD_MUL			3
#define CMD_DIV			4
#define CMD_SINCOS		5
#define CMD_FASTSINCOS	6
#define CMD_ATAN2			7
#define CMD_DIST			8
#define CMD_ASIN			9
#define CMD_ACOS			10
#define CMD_SQRT			11
#define CMD_ATOFIX24		12
#define CMD_HELP			98
#define CMD_QUIT			99

#define NUM_CMD_STRS		27

command cmd_list[] =
{
	"+",				CMD_ADD,
	"add",			CMD_ADD,

	"-",				CMD_SUB,
	"sub",			CMD_SUB,

	"*", 				CMD_MUL,
	"mul",			CMD_MUL,

	"/",				CMD_DIV,
	"div",			CMD_DIV,

	"sc",				CMD_SINCOS,
	"sincos",		CMD_SINCOS,

	"fsc",			CMD_FASTSINCOS,
	"fastsincos",	CMD_FASTSINCOS,

	"as",				CMD_ASIN,
	"asin",			CMD_ASIN,

	"ac",				CMD_ACOS,
	"acos",			CMD_ACOS,

	"a",				CMD_ATAN2,
	"a2",				CMD_ATAN2,
	"atan2",			CMD_ATAN2,

	"d",				CMD_DIST,
	"dist",			CMD_DIST,

	"sqrt",			CMD_SQRT,

	"af",				CMD_ATOFIX24,

	"?",				CMD_HELP,
	"help",			CMD_HELP,

	"q",				CMD_QUIT,
	"quit",			CMD_QUIT,
	"exit",			CMD_QUIT
};

bool check_args (int num)
{
	if (num_args >= num)
		return TRUE;
	else
	{
		printf ("Need %d args\n", num);
		return FALSE;
	}
}

// okay, this is now hairy enough that I should probably comment it
void parse (char *str, bool command)
{
	char *c;										// pointer to current char in str
	long val;										// value of current arg
	bool neg = FALSE;							// is this arg negative?
	bool frac = FALSE;						// is this arg really a fraction (after decimal point)?
	int i;										// counter
	int den;										// denominator of fraction

	// Prepare for death
	num_args = 0;
	c = str;
	while (isspace(*c)) c++;
	if (command) cmd = CMD_NONE;
	if (*c == NULL) return;

	// Look for matching commands, and then skip over it
	if (command)
	{
		for (i = 0; i < NUM_CMD_STRS; i++)
		{
			if (strnicmp (cmd_list[i].str, c, strlen (cmd_list[i].str)) == 0 &&
				  isspace(*(c+strlen(cmd_list[i].str))))
				break;
		}

		if (i < NUM_CMD_STRS)
		{
			c += strlen (cmd_list[i].str);
			cmd = cmd_list[i].cmd;
		}
		else
			return;
	}
	while (isspace(*c)) c++;

	// Stupid first time around stuff
	val = 0;
	if (*c == '-')
	{
		args_neg[0] = neg = TRUE; c++;
	}
	else
		args_neg[0] = neg = FALSE;

	while (*c != NULL)
	{
		// We have now gotten to the next non-whitespace char
		if (isdigit(*c))
		{
			// Update our numbers
			val = val * 10 + (*c++ - '0');
			den *= 10;
		}
		else
		{
			val *= (neg ? -1 : 1);
			if (frac)
			{
				val = (val << 8) / den;	// convert to fraction of 0x100
				// Hoo boy, is this ugly
				// If the user inputs -4.75, that's really an integer part of -5
				// and a fractional part of .25.  So deal accordingly.
				if (args_neg[num_args-1] && val != 0)
				{
					args[num_args-1] --;
					val = 0xff - val + 1;
				}
			}
			args[num_args++] = val;			// store the val away

			// are we about to do a decimal part?
			if (*c == '.')
			{
				frac = TRUE;
				den = 1;
			}
			else
				frac = FALSE;

			// prepare for the next argument
			c++;
			args_neg[num_args] = neg = FALSE;
			if (num_args == MAX_ARGS) break;
			while (isspace(*c)) c++;
			val = 0;
			if (*c == '-')
			{
				args_neg[num_args] = neg = TRUE; c++;
			}
		}	
	}

//	for (i = 0; i < num_args; i++)
//		printf ("%d ", args[i]);
//	printf ("\n");
}

void test_add (void)
{
	fix24 a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = a + b;
	fix24_sprint (ans, c);
	puts (ans);
}

void test_sub (void)
{
	fix24 a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = a - b;
	fix24_sprint (ans, c);
	puts (ans);
}

void test_mul (void)
{
	fix24 a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = fix24_mul (a, b);
	fix24_sprint (ans, c);
	puts (ans);
}

void test_div (void)
{
	fix24 a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = fix24_div (a, b);
	fix24_sprint (ans, c);
	puts (ans);
}

void test_sincos (void)
{
	fixang th;
	fix24 s, c;
	char sstr[80], cstr[80];

	if (!check_args(1)) return;
	th = args[0] * 0x10000 / 360;
	fix24_sincos (th, &s, &c);
	fix24_sprint (sstr, s);
	fix24_sprint (cstr, c);
	printf ("sin %s cos %s\n", sstr, cstr);
}

void test_fastsincos (void)
{
	fixang th;
	fix24 s, c;
	char sstr[80], cstr[80];

	if (!check_args(1)) return;
	th = args[0] * 0x10000 / 360;
	fix24_fastsincos (th, &s, &c);
	fix24_sprint (sstr, s);
	fix24_sprint (cstr, c);
	printf ("sin %s cos %s\n", sstr, cstr);
}

void test_atan2 (void)
{
	fix24 a, b, c;

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = fix24_atan2 (a, b);
	printf ("%04x\n", c);
}

void test_dist (void)
{
	fix24 a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = fix24_pyth_dist (a, b);
	fix24_sprint (ans, c);
	puts (ans);
}

void test_asin (void)
{
	fix24 a, c;

	if (!check_args(2)) return;
	a = fix24_make (args[0],args[1]);
	c = fix24_asin (a);
	printf ("%04x\n", c);
}

void test_acos (void)
{
	fix24 a, c;

	if (!check_args(2)) return;
	a = fix24_make (args[0],args[1]);
	c = fix24_acos (a);
	printf ("%04x\n", c);
}

void test_sqrt (void)
{
	fix24 a, c;
	char ans[80];

	if (!check_args(2)) return;
	a = fix24_make (args[0],args[1]);
	c = fix24_sqrt (a);
	fix24_sprint (ans, c);
	puts (ans);
}

void test_atofix24(void)
{
	fix24 a;
	char buff[80];

	printf("Enter number: ");
	gets(buff);
	a = atofix24(buff);
	fix24_sprint(buff, a);
	puts(buff);
}


void help ()
{
	printf ("Enter all numbers with decimal point, e.g. 5.0, -2.57\n");
	printf ("Functions:\n");
	printf ("  + a.b c.d\n");
	printf ("  - a.b c.d\n");
	printf ("  * a.b c.d\n");
	printf ("  / a.b c.d\n");
	printf ("  sincos a (in degrees)\n");
	printf ("  fastsincos a (in degrees)\n");
	printf ("  asin a.b\n");
	printf ("  acos a.b\n");
	printf ("  atan2 a.b c.d (y x)\n");
	printf ("  dist a.b c.d (sqrt ((a.b)^2 + (c.d^2)))\n");
	printf ("  sqrt a.b\n");
}

void main ()
{
	char ans[80];

	while (TRUE)
	{	
		printf (": ");
		fgets (ans, sizeof(ans), stdin);
		parse (ans, TRUE);
		if (cmd != CMD_NONE)
		{
			switch (cmd)
			{
			case CMD_ADD:			test_add ();			break;
			case CMD_SUB:			test_sub ();			break;
			case CMD_MUL:			test_mul ();			break;
			case CMD_DIV:			test_div ();			break;
			case CMD_SINCOS:		test_sincos ();		break;
			case CMD_FASTSINCOS:	test_fastsincos ();	break;
			case CMD_ATAN2:		test_atan2();			break;
			case CMD_DIST:			test_dist();			break;
			case CMD_ASIN:			test_asin();			break;
			case CMD_ACOS:			test_acos();			break;
			case CMD_SQRT:			test_sqrt();			break;
			case CMD_ATOFIX24:	test_atofix24();		break;
			case CMD_HELP:			help();					break;
			case CMD_QUIT:			exit (0);
			default:	printf ("Does not compute\n");	break;
			}
		}
	}
}
