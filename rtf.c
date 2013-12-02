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
		case TABLECAPTION:
		case MATHSPAN:
		case VERBATIM:
		case CODE:
		case BLOCKQUOTE:
			g_string_append_printf(out,"%s",n->str);
			break;
		case METADATA:
			print_text_node_tree(out,n->children,scratch);
			break;
		case METAKEY:
			g_string_append_printf(out,"%s:\t",n->str);
			print_text_node(out,n->children,scratch);
			break;
		case METAVALUE:
			g_string_append_printf(out,"%s",n->str);
			pad(out,1, scratch);
			break;
		case PARA:
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\\\n\\\n");
			break;
		case LIST:
		case HEADINGSECTION:
		/* TODO: Some of the following need improvements */
		case TABLE:
		case TABLECELL:
		case TABLEBODY:
		case TABLEROW:
			print_rtf_node_tree(out,n->children,scratch);
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
	g_string_append_printf(out, "{\\rtf1\n\n");
}

void end_rtf_output(GString *out, node* list, scratch_pad *scratch) {
	g_string_append_printf(out, "}\n");}
