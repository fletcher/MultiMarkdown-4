/*

	transclude.c -- miscellaneous support functions

	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "transclude.h"
#include "parser.h"


/* Combine directory and filename to create a full path */
char * path_from_dir_base(char *dir, char *base) {
#if defined(__WIN32)
	char sep = '\\';
#else
	char sep = '/';
#endif
	GString *path = NULL;
	char *result;

	if ((base != NULL) && (base[0] == sep)) {
		path = g_string_new(base);
	} else {
		path = g_string_new(dir);

		/* Ensure that folder ends in "/" */
		if (!(path->str[strlen(path->str)-1] == sep) ) {
			g_string_append_c(path, sep);
		}

		g_string_append_printf(path, "%s", base);
	}

	result = path->str;
	g_string_free(path, false);

	return result;
}

/* Return pointer to beginning of text without metadata */
char * source_without_metadata(char * source, unsigned long extensions ) {
	char *result;

	if (has_metadata(source, extensions)) {
		/* If we have metadata, then return just the body */
		result = strstr(source, "\n\n");

		if (result == NULL)
			return source;
		else 
			return result;
	}

	/* No metadata, so return original pointer */
	return source;
}

/* Given a GString containing MMD source, and optional base directory,
	substitute transclusion references in the source 

	Pass the path to the current folder if available -- should be a full path. 

	Keep track of what we're parsing to prevent recursion using stack. */
void transclude_source(GString *source, char *basedir, char *stack) {
	char *base = strdup(basedir);
	char *path = strdup(base);
	char *start;
	char *stop;
	char *temp;
	int curchar;
	size_t pos;
	char real[1000];
	FILE *input;

	GString *folder = NULL;
	GString *filename = NULL;
	GString *filebuffer = NULL;
	GString *stackstring = NULL;

	/* Change to file directory */
	if (base == NULL) {
		base = strdup("");
		path = strdup(base);
	}

	/* Look for override folder inside document */
	if (has_metadata(source->str, 0x000000)) {
		char *meta = extract_metadata_value(source->str, 0x000000, "transcludebase");
		if (meta != NULL)
			path = path_from_dir_base(base, meta);
	}

	if (path == NULL) {
		/* We have nowhere to look, so nothing to do */
		free(path);
		free(base);
		return;
	}

	folder = g_string_new(path);

	/* Ensure that folder ends in "/" */
	/* TODO: adjust for windows */
	if (!(folder->str[strlen(folder->str)-1] == '/') ) {
		g_string_append_c(folder, '/');
	}

	//fprintf(stderr, "Transclude using '%s'\n", folder->str);

	/* Iterate through {{foo.txt}} and substitute contents of file without metadata */

	start = strstr(source->str,"{{");

	while (start != NULL) {
		stop = strstr(start,"}}");
		if (stop == NULL)
			break;

		// TODO: Need to check that we found something reasonable 

		strncpy(real,start+2,stop-start-2);
		real[stop-start-2] = '\0';

		filename = g_string_new(folder->str);
		g_string_append_printf(filename, "%s",real);

		pos = stop - source->str;

		/* Don't reparse ourselves */
		if (stack != NULL) {
			temp = strstr(stack,filename->str);

			if ((temp != NULL) && (temp[strlen(filename->str)] == '\n')){
				start = strstr(source->str + pos,"{{");
				g_string_free(filename, true);
				continue;
			}
		}

		/* Read file */
		if ((input = fopen(filename->str, "r")) != NULL ) {
			filebuffer = g_string_new("");

			while ((curchar = fgetc(input)) != EOF)
				g_string_append_c(filebuffer, curchar);
			
			fclose(input);

 			pos = start - source->str;

			g_string_erase(source, pos, 2 + stop - start);

			/* Update stack list */
			stackstring = g_string_new(stack);
			g_string_append_printf(stackstring,"%s\n",filename->str);


			/* Recursively transclude files */
			transclude_source(filebuffer, folder->str, stackstring->str);

			temp = source_without_metadata(filebuffer->str, 0x000000);

			g_string_insert(source, pos, temp);

			pos += strlen(temp);
			g_string_free(filebuffer, true);
			g_string_free(stackstring, true);
		}

		start = strstr(source->str + pos,"{{");
		g_string_free(filename, true);
	}

	g_string_free(folder, true);
	free(path);
	free(base);
}
