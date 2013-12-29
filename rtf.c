/*

	rtf.c -- RTF writer

	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "rtf.h"


/* Since RTF sucks, we simplify here */

#define kNormalStyle "\\s0 \\qj\\sa180\\f0\\fs24 "
#define kH1 "\\s1 \\f1\\fs32\\ql\\sb240\\sa180\\b "
#define kH2 "\\s2 \\f1\\fs28\\ql\\sb240\\sa180\\i\\b "
#define kH3 "\\s3 \\f1\\fs28\\ql\\sb240\\sa180\\b "
#define kH4 "\\s4 \\f1\\fs24\\ql\\sb240\\sa180\\i\\b "
#define kH5 "\\s5 \\f1\\fs24\\ql\\sb240\\sa180\\b "
#define kH6 "\\s6 \\f1\\fs22\\ql\\sb240\\sa180\\b "

/* print_rtf_node_tree -- convert node tree to RTF */
void print_rtf_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_rtf_node(out, list, scratch);
		list = list->next;
	}
}

/* print_rtf_node -- convert given node to RTF and append */
void print_rtf_node(GString *out, node *n, scratch_pad *scratch) {
	int i;
	int lev;
	char *temp;

	switch (n->key) {
		case SPACE:
		case STR:
		/* TODO: Some of the following need improvements */
		case MATHSPAN:
		case VERBATIM:
		case CODE:
		case BLOCKQUOTE:
			print_rtf_string(out, n->str, scratch);
			break;
		case METADATA:
			g_string_append_printf(out, "{\\info\n");
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "}\n");
			scratch->padded = 0;
			break;
		case METAKEY:
			/* Convert key */
			temp = label_from_string(n->str);
			free(n->str);
			n->str = temp;
			fprintf(stderr, "%s\n", n->str);
			if (strcmp(n->str, "baseheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
				break;
			} else if (strcmp(n->str, "rtfheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
				break;
			} else if (strcmp(n->str, "quoteslanguage") == 0) {
				temp = label_from_node_tree(n->children);
				if ((strcmp(temp, "nl") == 0) || (strcmp(temp, "dutch") == 0)) { scratch->language = DUTCH; }   else 
				if ((strcmp(temp, "de") == 0) || (strcmp(temp, "german") == 0)) { scratch->language = GERMAN; } else 
				if (strcmp(temp, "germanguillemets") == 0) { scratch->language = GERMANGUILL; } else 
				if ((strcmp(temp, "fr") == 0) || (strcmp(temp, "french") == 0)) { scratch->language = FRENCH; } else 
				if ((strcmp(temp, "sv") == 0) || (strcmp(temp, "swedish") == 0)) { scratch->language = SWEDISH; }
				free(temp);
				break;
			}
	
			if (strcmp(n->str, "title") == 0) {
				g_string_append_printf(out, "{\\title ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "author") == 0) {
				g_string_append_printf(out, "{\\author ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "affiliation") == 0) {
				g_string_append_printf(out, "{\\company ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "company") == 0) {
				g_string_append_printf(out, "{\\company ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "keywords") == 0) {
				g_string_append_printf(out, "{\\keywords ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "copyright") == 0) {
				g_string_append_printf(out, "{\\*\\copyright ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "comment") == 0) {
				g_string_append_printf(out, "{\\doccom ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			}
			break;
		case METAVALUE:
			trim_trailing_whitespace(n->str);
			print_rtf_string(out, n->str, scratch);
			break;
		case PARA:
			pad_rtf(out, 2, scratch);
			g_string_append_printf(out, "{\\pard " kNormalStyle);
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\n\\par}\n");
			scratch->padded = 1;
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;
			if (lev > 6)
				lev = 6;
			pad_rtf(out, 2, scratch);
			switch (lev) {
				case 1:
					g_string_append_printf(out, "{\\pard " kH1);
					break;
				case 2:
					g_string_append_printf(out, "{\\pard " kH2);
					break;
				case 3:
					g_string_append_printf(out, "{\\pard " kH3);
					break;
				case 4:
					g_string_append_printf(out, "{\\pard " kH4);
					break;
				case 5:
					g_string_append_printf(out, "{\\pard " kH5);
					break;
				case 6:
					g_string_append_printf(out, "{\\pard " kH6);
					break;
			}
			print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\par}\n");
			scratch->padded = 1;
			break;
		case TABLE:
			pad_rtf(out, 2, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\pard\\par\n");
			scratch->padded = 1;
			break;
		case TABLESEPARATOR:
			scratch->table_alignment = n->str;
			break;
		case TABLEHEAD:
			scratch->cell_type = 'h';
			print_rtf_node_tree(out, n->children, scratch);
			scratch->cell_type = 'd';
			break;
		case TABLEROW:
			g_string_append_printf(out, "\\trowd\\trautofit1\n");
			for (i=0; i < strlen(scratch->table_alignment); i++) {
				g_string_append_printf(out, "\\cellx%d\n",i+1);
			}
			print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\row\n");
			break;
		case TABLECELL:
			g_string_append_printf(out, "\\intbl{");
			print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "}\\cell\n");
			break;
		case STRONG:
			g_string_append_printf(out, "{\\b ");
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "}");
			break;
		case EMPH:
			g_string_append_printf(out, "{\\i ");
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "}");
			break;
		case LINEBREAK:
			g_string_append_printf(out, "\\line ");
			break;
		case APOSTROPHE:
			print_rtf_localized_typography(out, APOS, scratch);
			break;
		case ELLIPSIS:
			print_rtf_localized_typography(out, ELLIP, scratch);
			break;
		case EMDASH:
			print_rtf_localized_typography(out, MDASH, scratch);
			break;
		case ENDASH:
			print_rtf_localized_typography(out, NDASH, scratch);
			break;
		case SINGLEQUOTED:
			print_rtf_localized_typography(out, LSQUOTE, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			print_rtf_localized_typography(out, RSQUOTE, scratch);
			break;
		case DOUBLEQUOTED:
			print_rtf_localized_typography(out, LDQUOTE, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			print_rtf_localized_typography(out, RDQUOTE, scratch);
			break;
		case LIST:
		case HEADINGSECTION:
			print_rtf_node_tree(out,n->children,scratch);
			break;
		/* TODO: Some of the following need improvements */
		case TABLEBODY:
		case TABLECAPTION:
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\\\n");
			break;
		case FOOTER:
			break;
		default:
			fprintf(stderr, "print_rtf_node encountered unknown node key = %d\n",n->key);
			g_string_append_printf(out, "%s",n->str);
			/* Will use in place of real output during development */
			/* exit(EXIT_FAILURE); */
			break;
	}
}

void begin_rtf_output(GString *out, node* list, scratch_pad *scratch) {
	g_string_append_printf(out, "{\\rtf1\\ansi\\deff0 {\\fonttbl\n" \
		"{\\f0\\froman Times;}\n" \
		"{\\f1\\swiss Arial;}\n" \
		"}\n" \
		"{\\stylesheet\n" \
		"{" kNormalStyle "Normal;}\n" \
		"{" kH1 "Header 1;}\n" \
		"{" kH2 "Header 2;}\n" \
		"{" kH3 "Header 3;}\n" \
		"{" kH4 "Header 4;}\n" \
		"{" kH5 "Header 5;}\n" \
		"{" kH6 "Header 6;}\n" \
		"}\n");
}

void end_rtf_output(GString *out, node* list, scratch_pad *scratch) {
	g_string_append_printf(out, "}\n");}

/* print_rtf_localized_typography -- convert to "smart" typography */
void print_rtf_localized_typography(GString *out, int character, scratch_pad *scratch) {
	if (!extension(EXT_SMART, scratch->extensions)) {
		g_string_append_c(out, character);
		return;
	}
	switch (character) {
		case LSQUOTE:
			switch (scratch->language) {
				case SWEDISH:
					g_string_append_printf(out, "&#8217;");
					break;
				case FRENCH:
					g_string_append_printf(out,"&#39;");
					break;
				case GERMAN:
					g_string_append_printf(out,"&#8218;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#8250;");
					break;
				default:
					g_string_append_printf(out,"\\'91");
				}
			break;
		case RSQUOTE:
			switch (scratch->language) {
				case GERMAN:
					g_string_append_printf(out,"&#8216;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#8249;");
					break;
				default:
					g_string_append_printf(out,"\\'92");
				}
			break;
		case APOS:
			g_string_append_printf(out,"\\'27");
			break;
		case LDQUOTE:
			switch (scratch->language) {
				case DUTCH:
				case GERMAN:
					g_string_append_printf(out,"&#8222;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#187;");
					break;
				case FRENCH:
					g_string_append_printf(out,"&#171;");
					break;
				case SWEDISH:
					g_string_append_printf(out, "&#8221;");
					break;
				default:
					g_string_append_printf(out,"\\'93");
				}
			break;
		case RDQUOTE:
			switch (scratch->language) {
				case SWEDISH:
				case DUTCH:
					g_string_append_printf(out,"&#8221;");
					break;
				case GERMAN:
					g_string_append_printf(out,"&#8220;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#171;");
					break;
				case FRENCH:
					g_string_append_printf(out,"&#187;");
					break;
				default:
					g_string_append_printf(out,"\\'94");
				}
			break;
		case NDASH:
			g_string_append_printf(out,"\\'96");
			break;
		case MDASH:
			g_string_append_printf(out,"\\'97");
			break;
		case ELLIP:
			g_string_append_printf(out,"\\'85");
			break;
			default:;
	}
}

void print_rtf_string(GString *out, char *str, scratch_pad *scratch) {
	if (str == NULL)
		return;
	while (*str != '\0') {
		switch (*str) {
			case '\\':
				g_string_append_printf(out, "\\\\");
				break;
			case '{':
				g_string_append_printf(out, "\\{");
				break;
			case '}':
				g_string_append_printf(out, "\\}");
				break;
			default:
				g_string_append_c(out, *str);
		}
		str++;
	}
}

void pad_rtf(GString *out, int num, scratch_pad *scratch) {
	while (num-- > scratch->padded)
		g_string_append_printf(out, "\n");
	
	scratch->padded = num;
}
