/*

	memoir.c -- Memoir add-on to LaTeX writer

	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "memoir.h"

/* print_memoir_node_tree -- convert node tree to LaTeX */
void print_memoir_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_memoir_node(out, list, scratch);
		list = list->next;
	}
}

/* print_memoir_node -- convert given node to LaTeX and append */
void print_memoir_node(GString *out, node *n, scratch_pad *scratch) {

	/* If we are forcing a complete document, and METADATA isn't the first thing,
		we need to close <head> */
	if ((scratch->extensions & EXT_COMPLETE)
		&& !(scratch->extensions & EXT_HEAD_CLOSED) && 
		!((n->key == FOOTER) || (n->key == METADATA))) {
			pad(out, 2, scratch);
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
		}
	switch (n->key) {
		case VERBATIM:
		case VERBATIMFENCE:
			pad(out, 2, scratch);
			if ((n->children != NULL) && (n->children->key == VERBATIMTYPE)) {
				trim_trailing_whitespace(n->children->str);
				if (strlen(n->children->str) > 0) {
					g_string_append_printf(out, "\\begin{adjustwidth}{2.5em}{2.5em}\n\\begin{lstlisting}[language=%s]\n", n->children->str);
					print_raw_node(out, n);
					g_string_append_printf(out, "\n\\end{lstlisting}\n\\end{adjustwidth}");					
					scratch->padded = 0;
					break;
				}
			}
			g_string_append_printf(out, "\\begin{adjustwidth}{2.5em}{2.5em}\n\\begin{verbatim}\n\n");
			print_raw_node(out, n);
			g_string_append_printf(out, "\n\\end{verbatim}\n\\end{adjustwidth}");
			scratch->padded = 0;
			break;
		case HEADINGSECTION:
			print_memoir_node_tree(out, n->children, scratch);
			break;
		case DEFLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{description}");
			scratch->padded = 0;
			print_memoir_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\end{description}");
			scratch->padded = 0;
			break;
		case DEFINITION:
			pad(out, 2, scratch);
			scratch->padded = 2;
			print_memoir_node_tree(out, n->children, scratch);
			scratch->padded = 0;
			break;
		default:
			/* most things are not changed for memoir output */
			print_latex_node(out, n, scratch);
	}
}
