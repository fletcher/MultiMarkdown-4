/*

	transclude.h -- miscellaneous support functions

	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include "GLibFacade.h"

char *	source_without_metadata(char * source, unsigned long extensions);
void	transclude_source(GString *source, char *basedir, char *stack, int format, GString *manifest);
void	append_mmd_footer(GString *source);
void	prepend_mmd_header(GString *source);
