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
#if defined(__WIN32)
#include <windows.h>
#endif


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

/* Separate filename and directory from a full path */
/* See http://stackoverflow.com/questions/1575278/function-to-split-a-filepath-into-path-and-file */
void split_path_file(char** dir, char** file, char *path) {
    char *slash = path, *next;
#if defined(__WIN32)
	const char sep[] = "\\";
#else
	const char sep[] = "/";
#endif

    while ((next = strpbrk(slash + 1, sep))) slash = next;
    if (path != slash) slash++;
    *dir = my_strndup(path, slash - path);
    *file = strdup(slash);
}

/* Return pointer to beginning of text without metadata */
/* NOTE: This is not a new string, and does not need to be freed separately */
char * source_without_metadata(char * source, unsigned long extensions ) {
	char *result;

	if (has_metadata(source, extensions)) {
		/* If we have metadata, then return just the body */
		/* TODO: This could miss YAML Metadata that does not contain
			blank line afterwards */
		result = strstr(source, "\n\n");

		if (result != NULL)
			return result + 2;
	}

	/* No metadata, so return original pointer */

	/* But check for UTF-8 BOM and skip if present */
	if (strncmp(source, "\xef\xbb\xbf",3) == 0)
		return source + 3;

	return source;
}

/* Given a GString containing MMD source, and optional base directory,
	substitute transclusion references in the source 

	Pass the path to the current folder if available -- should be a full path. 

	Keep track of what we're parsing to prevent recursion using stack. */
void transclude_source(GString *source, char *basedir, char *stack, int output_format, GString *manifest) {
	char *base = NULL;
	char *path = NULL;
	char *start;
	char *stop;
	char *temp;
	int curchar;
	size_t pos;
	char real[1000];
	FILE *input;

	if (basedir == NULL) {
		base = strdup("");
	} else {
		base = strdup(basedir);
	}

	GString *folder = NULL;
	GString *filename = NULL;
	GString *filebuffer = NULL;
	GString *stackstring = NULL;

	path = strdup(base);

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

	/* fprintf(stderr, "Transclude using '%s'\n", folder->str); */

	/* Iterate through {{foo.txt}} and substitute contents of file without metadata */

	start = strstr(source->str,"{{");

	while (start != NULL) {
		stop = strstr(start,"}}");
		if (stop == NULL)
			break;

		/* Check that we found something reasonable -- we cap at 1000 characters */
        if (stop - start < 1000) {
			strncpy(real,start+2,stop-start-2);
			real[stop-start-2] = '\0';

			filename = g_string_new(folder->str);
			g_string_append_printf(filename, "%s",real);

			/* Adjust for wildcard extensions */
			/* But not if output_format == 0 */
			if (output_format && strncmp(&filename->str[strlen(filename->str) - 2],".*",2) == 0) {
				g_string_erase(filename, strlen(filename->str) - 2, 2);
				if (output_format == TEXT_FORMAT) {
					g_string_append(filename,".txt");
				} else if (output_format == HTML_FORMAT) {
					g_string_append(filename,".html");
				} else if (output_format == LATEX_FORMAT) {
					g_string_append(filename,".tex");
				} else if (output_format == BEAMER_FORMAT) {
					g_string_append(filename,".tex");
				} else if (output_format == MEMOIR_FORMAT) {
					g_string_append(filename,".tex");
				} else if (output_format == ODF_FORMAT) {
					g_string_append(filename,".fodt");
				} else if (output_format == OPML_FORMAT) {
					g_string_append(filename,".opml");
				} else if (output_format == LYX_FORMAT) {
					g_string_append(filename,".lyx");
				} else if (output_format == RTF_FORMAT) {
					g_string_append(filename,".rtf");
				} else {
					/* default extension -- in this case we only have 1 */
					g_string_append(filename,".txt");
				}
			}

			pos = stop - source->str;

			/* Add to the manifest (if not already included) */
			if (manifest != NULL) {
				temp = strstr(manifest->str,filename->str);

				if ((temp != NULL) && (temp[strlen(filename->str)] == '\n')){
					/* Already on manifest */
				} else {
					g_string_append_printf(manifest,"%s\n",filename->str);
				}
			}


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
#if defined(__WIN32)
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, filename->str, -1, NULL, 0);
			wchar_t wstr[wchars_num];
			MultiByteToWideChar(CP_UTF8, 0, filename->str, -1, wstr, wchars_num);

			if ((input = _wfopen(wstr, L"r")) != NULL ) {
#else
			if ((input = fopen(filename->str, "r")) != NULL ) {
#endif
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

				/* We want to reset the base directory if we enter a subdirectory */
				char * new_dir;
				char * file_only;
				split_path_file(&new_dir, &file_only, filename->str);

				/* transclude_source(filebuffer, folder->str, stackstring->str, output_format, manifest); */
				transclude_source(filebuffer, new_dir, stackstring->str, output_format, manifest);

				free(new_dir);
				free(file_only);
				
				temp = source_without_metadata(filebuffer->str, 0x000000);

				g_string_insert(source, pos, temp);

				pos += strlen(temp);
				g_string_free(filebuffer, true);
				g_string_free(stackstring, true);
			} else {
				/* fprintf(stderr, "error opening file: %s\n", filename->str); */
			}

            g_string_free(filename, true);
        } else {
        	/* Our "match" was > 1000 characters long */
            pos = stop - source->str;
        }
		start = strstr(source->str + pos,"{{");
	}

	g_string_free(folder, true);
	free(path);
	free(base);
}

/* Allow for a footer to specify files to be appended to the end of the text, and then transcluded.
	Useful for appending a list of footnotes, citations, abbreviations, etc. to each separate file,
	but not including multiple copies when processing the master file. */
void append_mmd_footer(GString *source) {
	/* Look for mmd_footer metadata */
	if (has_metadata(source->str, 0x000000)) {
		char *meta = extract_metadata_value(source->str, 0x000000, "mmdfooter");
		if (meta != NULL)
			g_string_append_printf(source, "\n\n{{%s}}\n", meta);
	}
}

void prepend_mmd_header(GString *source) {
	/* Same thing, but to be inserted after metadata and before content */
	if (has_metadata(source->str, 0x000000)) {
		char *meta = extract_metadata_value(source->str, 0x000000, "mmdheader");
		if (meta != NULL) {
			char *content = strstr(source->str, "\n\n");
			if (content != NULL) {
				size_t pos = content - source->str;
				g_string_insert_printf(source, pos, "\n\n{{%s}}", meta);
			} else {
				g_string_append_printf(source, "\n\n{{%s}}\n", meta);
			}
		}
	}
}
