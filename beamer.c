/*

	beamer.c -- Beamer add-on to LaTeX writer

	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "beamer.h"

/* print_beamer_node_tree -- convert node tree to LaTeX */
void print_beamer_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_beamer_node(out, list, scratch);
		list = list->next;
	}
}

/* print_beamer_node -- convert given node to LaTeX and append */
void print_beamer_node(GString *out, node *n, scratch_pad *scratch) {
	int lev;
	char *temp;
	
	/* If we are forcing a complete document, and METADATA isn't the first thing,
		we need to close <head> */
	if ((scratch->extensions & EXT_COMPLETE)
		&& !(scratch->extensions & EXT_HEAD_CLOSED) && 
		!((n->key == FOOTER) || (n->key == METADATA))) {
			pad(out, 2, scratch);
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
		}
	switch (n->key) {
		case FOOTER:
			print_beamer_endnotes(out, scratch);
			g_string_append_printf(out, "\\mode<all>\n");
			if (scratch->latex_footer != NULL) {
				pad(out, 2, scratch);
				g_string_append_printf(out,"\\input{%s}\n", scratch->latex_footer);
			}
			if (scratch->extensions & EXT_COMPLETE) {
				g_string_append_printf(out, "\n\\end{document}");
			}
			g_string_append_printf(out, "\\mode*\n");
			break;
		case LISTITEM:
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\item<+-> ");
			scratch->padded = 2;
			print_latex_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\n");
			break;
		case HEADINGSECTION:
			if (n->children->key -H1 + scratch->baseheaderlevel == 3) {
				pad(out, 2, scratch);
				g_string_append_printf(out, "\\begin{frame}");
				/* TODO: Fix this */
				if (tree_contains_key(n->children,VERBATIM) ||
				    tree_contains_key(n->children,VERBATIMFENCE)) {
					g_string_append_printf(out, "[fragile]");
				}
				scratch->padded = 0;
				print_beamer_node_tree(out, n->children, scratch);
				g_string_append_printf(out, "\n\n\\end{frame}\n\n");
				scratch->padded = 2;
			} else if (n->children->key -H1 + scratch->baseheaderlevel == 4) {
				pad(out, 1, scratch);
				g_string_append_printf(out, "\\mode<article>{\n");
				scratch->padded = 0;
				print_beamer_node_tree(out, n->children->next, scratch);
				g_string_append_printf(out, "\n\n}\n\n");
				scratch->padded = 2;
			} else {
				print_beamer_node_tree(out, n->children, scratch);
			}
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			pad(out, 2, scratch);
			lev = n->key - H1 + scratch->baseheaderlevel;  /* assumes H1 ... H6 are in order */
			switch (lev) {
				case 1:
					g_string_append_printf(out, "\\part{");
					break;
				case 2:
					g_string_append_printf(out, "\\section{");
					break;
				case 3:
					g_string_append_printf(out, "\\frametitle{");
					break;
				default:
					g_string_append_printf(out, "\\emph{");
					break;
			}
			/*  generate a label for each header (MMD);
				don't allow footnotes since invalid here */
			scratch->no_latex_footnote = TRUE;
			if (n->children->key == AUTOLABEL) {
				temp = label_from_string(n->children->str);
				print_latex_node_tree(out, n->children->next, scratch);
				g_string_append_printf(out, "}\n\\label{%s}", temp);
				free(temp);
			} else {
				print_latex_node_tree(out, n->children, scratch);
				temp = label_from_node_tree(n->children);
				g_string_append_printf(out, "}\n\\label{%s}", temp);
				free(temp);
			}
			scratch->no_latex_footnote = FALSE;
			scratch->padded = 0;
			break;
		default:
			/* most things are not changed for memoir output */
			print_latex_node(out, n, scratch);
	}
}

/* print_beamer_endnotes */
void print_beamer_endnotes(GString *out, scratch_pad *scratch) {
	scratch->used_notes = reverse_list(scratch->used_notes);
	scratch->printing_notes = 1;
	
	node *note = scratch->used_notes;
#ifdef DEBUG_ON
	fprintf(stderr, "start endnotes\n");
#endif
	
	if ((note == NULL) || ((note->key == KEY_COUNTER) && (note->next == NULL)))
		return;
	while (note != NULL) {
		if (note->key == CITATIONSOURCE)
			break;
		note = note->next;
	}

	if (note == NULL)
		return;
	
	note = scratch->used_notes;
	
	/* TODO: need CITATIONSOURCE to print bibliography */
#ifdef DEBUG_ON
	fprintf(stderr, "there are endnotes to print\n");
#endif

	pad(out, 2, scratch);
	g_string_append_printf(out, "\\part{Bibliography}\n\\begin{frame}[allowframebreaks]\n\\frametitle{Bibliography}\n\\def\\newblock{}\n\\begin{thebibliography}{0}\n");
	while ( note != NULL) {
		if (note->key == KEY_COUNTER) {
			note = note->next;
			continue;
		}
		
		pad(out, 1, scratch);
		
		if (note->key == CITATIONSOURCE) {
			g_string_append_printf(out, "\\bibitem{%s}\n", note->str);
			scratch->padded = 2;

			print_latex_node(out, note, scratch);
			pad(out, 1, scratch);
			scratch->padded = 1;
		} else {
			/* footnotes handled elsewhere */
		}

		note = note->next;
	}
	pad(out,2, scratch);
	g_string_append_printf(out, "\\end{thebibliography}\n\\end{frame}\n\n");
	scratch->padded = 0;
#ifdef DEBUG_ON
	fprintf(stderr, "finish endnotes\n");
#endif
}
