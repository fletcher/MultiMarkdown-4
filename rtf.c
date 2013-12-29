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

/* print_rtf_node_tree -- convert node tree to RTF */
void print_rtf_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_rtf_node(out, list, scratch);
		list = list->next;
	}
}

/* print_rtf_node -- convert given node to RTF and append */
void print_rtf_node(GString *out, node *n, scratch_pad *scratch) {
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
			print_text_node_tree(out,n->children,scratch);
			break;
		case METAKEY:
			print_rtf_string(out, n->str, scratch);
			g_string_append_printf(out,"\t");
			print_text_node(out,n->children,scratch);
			break;
		case METAVALUE:
			print_rtf_string(out, n->str, scratch);
			pad_rtf(out,1, scratch);
			break;
		case PARA:
			pad_rtf(out, 2, scratch);
			print_rtf_node_tree(out,n->children,scratch);
			scratch->padded = 0;
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			pad_rtf(out, 2, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			scratch->padded = 0;
			break;
		case TABLE:
			pad_rtf(out, 2, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			scratch->padded = 1;
			break;
		case TABLEROW:
			g_string_append_printf(out, "\\trowd\n");
			print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\row\n");
			break;
		case TABLECELL:
			g_string_append_printf(out, "\\intbl ");
			print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\cell\n");
			break;
		case STRONG:
			g_string_append_printf(out, "\\b ");
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\\b0 ");
			break;
		case EMPH:
			g_string_append_printf(out, "\\i ");
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\\i0 ");
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
		/* TODO: Some of the following need improvements */
		case TABLEBODY:
		case TABLECAPTION:
			print_rtf_node_tree(out,n->children,scratch);
			break;
		case TABLEHEAD:
		case FOOTER:
		case TABLESEPARATOR:
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
	g_string_append_printf(out, "{\\rtf1\\ansi\n\n");
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
		g_string_append_printf(out, "\\\n");
	
	scratch->padded = num;
}
