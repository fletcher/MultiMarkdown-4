/*

	parser.leg -> parser.c -- Parse (Multi)Markdown plain text for
		conversion into other formats
	
	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	Derived from peg-multimarkdown, which was forked from peg-markdown,
	which is (c) 2008 John MacFarlane (jgm at berkeley dot edu), and 
	licensed under GNU GPL or MIT.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include <getopt.h>
#include <libgen.h>
#include "parser.h"
#include "transclude.h"

int main(int argc, char **argv)
{
	int numargs;
	int c;
	int i;
	static int batch_flag = 0;
	static int complete_flag = 0;
	static int snippet_flag = 0;
	static int compatibility_flag = 0;
	static int notes_flag = 0;
	static int no_notes_flag = 0;
	static int typography_flag = 0;
	static int no_typography_flag = 0;
	static int label_flag = 0;
	static int no_label_flag = 0;
	static int escaped_line_breaks_flag = 0;
	static int obfuscate_flag = 0;
	static int no_obfuscate_flag = 0;
	static int process_html_flag = 0;
	static int random_footnotes_flag = 0;
	bool list_meta_keys = 0;
	bool list_transclude_manifest = 0;
	char *target_meta_key = FALSE;
		
	static struct option long_options[] = {
		{"batch", no_argument, &batch_flag, 1},                              /* process each file separately */
		{"to", required_argument, 0, 't'},                                   /* which output format to use */
		{"full", no_argument, &complete_flag, 1},                            /* complete document */
		{"snippet", no_argument, &snippet_flag, 1},                          /* snippet only */
		{"output", required_argument, 0, 'o'},                               /* which output format to use */
		{"notes", no_argument, &notes_flag, 1},                              /* use footnotes */
		{"nonotes", no_argument, &no_notes_flag, 1},                         /* don't use footnotes */
		{"smart", no_argument, &typography_flag, 1},                         /* use smart typography */
		{"nosmart", no_argument, &no_typography_flag, 1},                    /* don't use smart typography */
		{"mask", no_argument, &obfuscate_flag, 1},                           /* mask email addresses */
		{"nomask", no_argument, &no_obfuscate_flag, 1},                      /* don't mask email addresses */
		{"labels", no_argument, &label_flag, 1},                             /* generate labels */
		{"nolabels", no_argument, &no_label_flag, 1},                        /* don't generate labels */
		{"escaped-line-breaks", no_argument, &escaped_line_breaks_flag, 1},  /* enable escaped line breaks */
		{"compatibility", no_argument, &compatibility_flag, 1},              /* compatibility mode */
		{"process-html", no_argument, &process_html_flag, 1},                /* process Markdown inside HTML */
		{"random", no_argument, &random_footnotes_flag, 1},                  /* Use random numbers for footnote links */
		{"accept", no_argument, 0, 'a'},                                     /* Accept all proposed CriticMarkup changes */
		{"reject", no_argument, 0, 'r'},                                     /* Reject all proposed CriticMarkup changes */
		{"metadata-keys", no_argument, 0, 'm'},                              /* List all metadata keys */
		{"extract", required_argument, 0, 'e'},                              /* show value of specified metadata */
		{"version", no_argument, 0, 'v'},                                    /* display version information */
		{"help", no_argument, 0, 'h'},                                       /* display usage information */
		{"manifest", no_argument, 0, 'x'},                                   /* List all transcluded files */
		{NULL, 0, NULL, 0}
	};
	
	GString *inputbuf;
	GString *manifest;
	FILE *input;
	FILE *output;
	int curchar;
	GString *filename = NULL;
	
	char *out;
	
	/* set up my data for the parser */
	int output_format = 0;
	unsigned long extensions = 0;
	extensions = extensions | EXT_SMART | EXT_NOTES | EXT_OBFUSCATE;
	
	/* process options */
	while (1) {
		int option_index = 0;

		c = getopt_long (argc, argv, "vhco:bfst:me:arx", long_options, &option_index);
		
		if (c == -1)
			break;
		
		switch (c) {
			case 0: /* handle long_options */
				/* printf ("option %s", long_options[option_index].name);
				if (optarg)
				printf (" with arg %s", optarg);
				printf("\n"); */
				break;
			
			case 'b':	/* batch */
				batch_flag = 1;
				break;
			
			case 'c':	/* compatibility */
				compatibility_flag = 1;
				break;
			
			case 'o':	/* output filename */
				if (optarg)
					filename = g_string_new(optarg);
				break;
			
			case 'v':	/* show version */
				printf("\nMultiMarkdown version %s\n%s\n",MMD_VERSION, MMD_COPYRIGHT);
				return(EXIT_SUCCESS);

			case 'h':	/* show usage */
				printf("\nMultiMarkdown version %s\n\n",MMD_VERSION);
				printf("    %s  [OPTION...] [FILE...]\n",argv[0]);
				printf("\n"
				"    Options:\n"
				"    -h, --help             Show help\n"
				"    -v, --version          Show version information\n"
				"    -o, --output=FILE      Send output to FILE\n"
				"    -t, --to=FORMAT        Convert to FORMAT\n"
				"    -b, --batch            Process each file separately\n"
				"    -c, --compatibility    Markdown compatibility mode\n"
				"    -f, --full             Force a complete document\n"
				"    -s, --snippet          Force a snippet\n"
				"    --process-html         Process Markdown inside of raw HTML\n"
				"    -m, --metadata-keys    List all metadata keys\n"
				"    -e, --extract          Extract specified metadata\n"
				"    -x, --manifest         Show manifest of all transcluded files\n"
				"    --random               Use random numbers for footnote anchors\n"
				"\n"
				"    -a, --accept           Accept all CriticMarkup changes\n"
				"    -r, --reject           Reject all CriticMarkup changes\n"
				"\n"
				"    --smart, --nosmart     Toggle smart typography\n"
				"    --notes, --nonotes     Toggle footnotes\n"
				"    --labels, --nolabels   Disable id attributes for headers\n"
				"    --mask, --nomask       Mask email addresses in HTML\n"
				"    --escaped-line-breaks  Enable escaped line breaks\n"

				"\nAvailable FORMATs: html(default), latex, beamer, memoir, odf, opml, lyx, mmd\n\n"
				"NOTE: The lyx output format was created by Charles R. Cowan, and \n\tis provided as is.\n\n\n"
				);
				return(EXIT_SUCCESS);
			
			case 't':	/* output format */
				if (strcmp(optarg, "text") == 0)
					output_format = TEXT_FORMAT;
				else if (strcmp(optarg, "html") == 0)
					output_format = HTML_FORMAT;
				else if (strcmp(optarg, "latex") == 0)
					output_format = LATEX_FORMAT;
				else if (strcmp(optarg, "memoir") == 0)
					output_format = MEMOIR_FORMAT;
				else if (strcmp(optarg, "beamer") == 0)
					output_format = BEAMER_FORMAT;
				else if (strcmp(optarg, "opml") == 0)
					output_format = OPML_FORMAT;
				else if (strcmp(optarg, "odf") == 0)
					output_format = ODF_FORMAT;
				else if (strcmp(optarg, "rtf") == 0)
					output_format = RTF_FORMAT;
				else if (strcmp(optarg, "lyx") == 0)
					output_format = LYX_FORMAT;
				else if (strcmp(optarg, "mmd") == 0)
					output_format = ORIGINAL_FORMAT;
				else {
					/* no valid format specified */
					fprintf(stderr, "%s: Unknown output format '%s'\n",argv[0], optarg);
					exit(EXIT_FAILURE);
				}
				break;
			
			case 'f':	/* full doc */
				extensions = extensions | EXT_COMPLETE;
				break;
			
			case 's':	/* snippet only */
				extensions = extensions | EXT_SNIPPET;
				break;
			
			case 'm':	/* list metadata */
				list_meta_keys = 1;
				break;

			case 'e':	/* extract metadata */
				target_meta_key = strdup(optarg);
				break;
			
			case '?':	/* long handles */
				break;
				
			case 'a':	/* Accept CriticMarkup changes */
				extensions = extensions | EXT_CRITIC_ACCEPT;
				break;
			
			case 'r':	/* Reject CriticMarkup changes */
				extensions = extensions | EXT_CRITIC_REJECT;
				break;
			
			case 'x':	/* List transcluded files */
				list_transclude_manifest = 1;
				break;

			default:
			fprintf(stderr,"Error parsing options.\n");
			abort();
		}
	}	
	
	/* Compatibility mode emulates the behavior of Markdown.pl */
	if (compatibility_flag) {
		extensions = 0x000000;
		extensions = extensions | EXT_COMPATIBILITY | EXT_NO_LABELS | EXT_OBFUSCATE;
	}

	/* apply extensions from long options*/
	if (complete_flag)
		extensions = extensions | EXT_COMPLETE;
	
	if (snippet_flag)
		extensions = extensions | EXT_SNIPPET;
	
	if (notes_flag)
		extensions = extensions | EXT_NOTES;
	
	if (no_notes_flag)
		extensions &= ~EXT_NOTES;

	if (typography_flag)
		extensions = extensions | EXT_SMART;

	if (no_typography_flag)
		extensions &= ~EXT_SMART;

	if (label_flag)
		extensions &= ~EXT_NO_LABELS;

	if (no_label_flag)
		extensions = extensions | EXT_NO_LABELS;
	
	if (obfuscate_flag)
		extensions = extensions | EXT_OBFUSCATE;

	if (no_obfuscate_flag)
		extensions &= ~EXT_OBFUSCATE;

	if (process_html_flag)
		extensions = extensions | EXT_PROCESS_HTML;
	
	if (random_footnotes_flag)
		extensions = extensions | EXT_RANDOM_FOOT;

	if (escaped_line_breaks_flag)
		extensions = extensions | EXT_ESCAPED_LINE_BREAKS;

	/* Enable HEADINGSECTION for certain formats */
	if ((output_format == OPML_FORMAT) || (output_format == BEAMER_FORMAT) || (output_format == LYX_FORMAT))
		extensions = extensions | EXT_HEADINGSECTION;
	
	/* fix numbering to account for options */
	argc -= optind;
	argv += optind;
	
	/* We expect argc and argv to still point just one below the start of remaining args */
	argc++;
	argv--;
	
	/* any filenames */
	numargs = argc -1;

	if (batch_flag && (numargs != 0)) {
		/* we have multiple file names -- handle individually */
		
		for (i = 0; i < numargs; i++) {
			inputbuf = g_string_new("");
			manifest = g_string_new("");
			char *temp = NULL;
			char *folder = NULL;

			/* Read file */
			if ((input = fopen(argv[i+1], "r")) == NULL ) {
				perror(argv[i+1]);
				g_string_free(inputbuf, true);
				g_string_free(filename, true);
				exit(EXIT_FAILURE);
			}
			
			while ((curchar = fgetc(input)) != EOF)
				g_string_append_c(inputbuf, curchar);
			fclose(input);
			
			/* list metadata keys */
			if (list_meta_keys) {
				out = extract_metadata_keys(inputbuf->str, extensions);
				if (out != NULL) {
					fprintf(stdout, "%s", out);
					free(out);
					g_string_free(inputbuf, true);
					free(target_meta_key);
					return(EXIT_SUCCESS);
				}
			}
			
			/* extract metadata */
			if (target_meta_key) {
				out = extract_metadata_value(inputbuf->str, extensions, target_meta_key);
				if (out != NULL)
					fprintf(stdout, "%s\n", out);
				free(out);
				g_string_free(inputbuf, true);
				free(target_meta_key);
				return(EXIT_SUCCESS);
			}
			
			if (!(extensions & EXT_COMPATIBILITY)) {
				temp = strdup(argv[i+1]);
				folder = dirname(temp);
				prepend_mmd_header(inputbuf);
				append_mmd_footer(inputbuf);
				transclude_source(inputbuf, folder, NULL, output_format, manifest);
				free(temp);
				// free(folder);
			}

			/* list transclude manifest */
			if (list_transclude_manifest) {
				fprintf(stdout, "%s\n", manifest->str);
				g_string_free(inputbuf, true);
				g_string_free(manifest, true);
				return(EXIT_SUCCESS);
			} else {
				g_string_free(manifest, true);
			}

			if (output_format == ORIGINAL_FORMAT) {
				/* We want the source, don't parse */
				out = (inputbuf->str);
				g_string_free(inputbuf, FALSE);
			} else {
				out = markdown_to_string(inputbuf->str,  extensions, output_format);
				g_string_free(inputbuf, true);
			}
			
			/* set up for output */
			temp = argv[i+1];	/* get current filename */
			if (strrchr(temp,'.') != NULL) {
				long count = strrchr(temp,'.') - temp;
				if (count != 0) {
					/* truncate string at "." */
					temp[count] = '\0';
				}
			}
			
			filename = g_string_new(temp);
			
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
			} else if (output_format == ORIGINAL_FORMAT) {
				g_string_append(filename,".mmd_out");
			} else {
				/* default extension -- in this case we only have 1 */
				g_string_append(filename,".txt");
			}
			
			if (!(output = fopen(filename->str, "w"))) {
				perror(filename->str);
			} else {
				fprintf(output, "%s\n",out);
				fclose(output);
			}
			
			g_string_free(filename,true);
			
			if (out != NULL)
				free(out);
		}
	} else {
		/* get input from stdin or concat all files */
		inputbuf = g_string_new("");
		char *folder = NULL;
		char *temp = NULL;
		GString *manifest = g_string_new("");

		folder = getcwd(0,0);

		if (numargs == 0) {
			/* get stdin */
			while ((curchar = fgetc(stdin)) != EOF)
				g_string_append_c(inputbuf, curchar);
			fclose(stdin);
		} else {
			/* get files */
			free(folder);
			temp = strdup(argv[1]);
			folder = dirname(temp);

			for (i = 0; i < numargs; i++) {
				if ((input = fopen(argv[i+1], "r")) == NULL ) {
					perror(argv[i+1]);
					g_string_free(inputbuf, true);
					g_string_free(filename, true);
					// free(folder);
					free(temp);
					exit(EXIT_FAILURE);
				}
				
				while ((curchar = fgetc(input)) != EOF)
					g_string_append_c(inputbuf, curchar);
				fclose(input);
			}
		}
		
		if (!(extensions & EXT_COMPATIBILITY)) {
			prepend_mmd_header(inputbuf);
			append_mmd_footer(inputbuf);
			transclude_source(inputbuf, folder, NULL, output_format, manifest);
		}

		free(temp);

		//if (folder != NULL)
		//	free(folder);

		/* list metadata keys */
		if (list_meta_keys) {
			out = extract_metadata_keys(inputbuf->str, extensions);
			if (out != NULL) {
				fprintf(stdout, "%s", out);
				free(out);
				g_string_free(inputbuf, true);
				free(target_meta_key);
				return(EXIT_SUCCESS);
			}
		}

		/* extract metadata */
		if (target_meta_key) {
			out = extract_metadata_value(inputbuf->str, extensions, target_meta_key);
			if (out != NULL)
				fprintf(stdout, "%s\n", out);
			free(out);
			g_string_free(inputbuf, true);
			free(target_meta_key);
			return(EXIT_SUCCESS);
		}

		/* list transclude manifest */
		if (list_transclude_manifest) {
			fprintf(stdout, "%s\n", manifest->str);
			g_string_free(inputbuf, true);
			g_string_free(manifest, true);
			return(EXIT_SUCCESS);
		} else {
			g_string_free(manifest, true);			
		}

		if (output_format == ORIGINAL_FORMAT) {
			/* We want the source, don't parse */
			out = (inputbuf->str);
			g_string_free(inputbuf, FALSE);
		} else {
			out = markdown_to_string(inputbuf->str,  extensions, output_format);
			g_string_free(inputbuf, true);
		}
		
		/* did we specify an output filename; "-" equals stdout */
		if ((filename == NULL) || (strcmp(filename->str, "-") == 0)) {
			output = stdout;
		} else if (!(output = fopen(filename->str, "w"))) {
			perror(filename->str);
			if (out != NULL)
				free(out);
			g_string_free(filename, true);
			return 1;
		}
		
		fprintf(output, "%s\n",out);
		fclose(output);
		
		g_string_free(filename, true);
		
		if (out != NULL)
			free(out);
	}
	
	return(EXIT_SUCCESS);
}
