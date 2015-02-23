/*

	opml.c -- OPML add-on to LaTeX writer

	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "opml.h"

/* begin_opml_output -- handle the initial prefix, if any */
void begin_opml_output(GString *out, node* list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "begin_opml_output\n");
#endif
	node *title;
	
	g_string_append_printf(out, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<opml version=\"1.0\">\n");
	
	if (tree_contains_key(list, METAKEY)) {
		title = metadata_for_key("title", list);
		if (title != NULL) {
			char *temp_str;
			GString *temp = g_string_new("");
			g_string_append_printf(out, "<head><title>");
			print_raw_node_tree(temp, title->children);
			temp_str = strdup(temp->str);
			trim_trailing_whitespace(temp_str);
			print_opml_string(out, temp_str);
			g_string_append_printf(out, "</title></head>\n",temp_str);
			free(temp_str);
			g_string_free(temp, true);
		}
	}
	g_string_append_printf(out, "<body>\n");
}

/* end_opml_output -- close the document */
void end_opml_output(GString *out, node* list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "end_opml_output\n");
#endif
	if (tree_contains_key(list, METAKEY)) {
		g_string_append_printf(out, "<outline text=\"Metadata\">\n");
		print_opml_node_tree(out, list->children, scratch);
		g_string_append_printf(out, "</outline>");
	}
	g_string_append_printf(out, "</body>\n</opml>");
}

/* print_opml_node_tree -- convert node tree to LaTeX */
void print_opml_node_tree(GString *out, node *list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "print_opml_node_tree\n");
#endif
	int lev;
	while (list != NULL) {
		if (list->key == HEADINGSECTION) {
			lev = list->children->key;
			
			print_opml_section_and_children(out, list, scratch);
			
			while ((list->next != NULL) && (list->next->key == HEADINGSECTION)
				&& (list->next->children->key > lev)) {
					list = list->next;
				}
		} else {
			print_opml_node(out, list, scratch);
		}
		list = list->next;
	}
}

/* print_opml_section_and_children -- we want to stay inside the outline structure */
void print_opml_section_and_children(GString *out, node *list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "print_opml_section_and_children: %d\n",list->key);
#endif
	int lev = list->children->key;
	
	/* print current section (parent) */
	print_opml_node(out, list, scratch);
	
	/* check for child nodes */
	while ((list->next != NULL) && (list->next->key == HEADINGSECTION) && (list->next->children->key > lev)) {
		/* next item is also a HEADINGSECTION and is a child */
		if (list->next->children->key - lev == 1)
			print_opml_section_and_children(out, list->next, scratch);
		list = list->next;
	}
	g_string_append_printf(out, "</outline>\n");
}

/* print_opml_node -- convert given node to OPML and append */
void print_opml_node(GString *out, node *n, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "print_opml_node: %d\n",n->key);
#endif
	switch (n->key) {
		case METADATA:
			/* Metadata is present, so will need to be appended later */
			break;
		case METAKEY:
			g_string_append_printf(out, "<outline text=\"");
			print_opml_string(out, n->str);
			g_string_append_printf(out, "\" _note=\"");
			trim_trailing_newlines(n->children->str);
			print_opml_string(out, n->children->str);
			g_string_append_printf(out, "\"/>");
			break;
		case HEADINGSECTION:
			/* Need to handle "nesting" properly */
			g_string_append_printf(out, "<outline ");

			/* Print header */
			print_opml_node(out, n->children, scratch);

			/* print remainder of paragraphs as note */
			g_string_append_printf(out, " _note=\"");
			print_opml_node_tree(out, n->children->next, scratch);
			g_string_append_printf(out, "\">");
			break;
		case H1: case H2: case H3: case H4: case H5: case H6: 
			g_string_append_printf(out, "text=\"");
			print_opml_string(out, n->str);
			g_string_append_printf(out,"\"");
			break;
		case VERBATIM:
		case VERBATIMFENCE:
			print_opml_string(out, n->str);
			break;
		case SPACE:
			print_opml_string(out, n->str);
			break;
		case STR:
			print_opml_string(out, n->str);
			break;
		case LINEBREAK:
			g_string_append_printf(out, "  &#10;");
			break;
		case PLAIN:
			print_opml_node_tree(out, n->children, scratch);
			if ((n->next != NULL) && (n->next->key == PLAIN)) {
				g_string_append_printf(out, "&#10;");
			}
			break;
		default: 
			fprintf(stderr, "print_opml_node encountered unknown element key = %d\n", n->key);
			exit(EXIT_FAILURE);
	}
#ifdef DEBUG_ON
	fprintf(stderr, "finish print_opml_node: %d\n", n->key);
#endif
}

/* print_opml_string - print string, escaping for OPML */
void print_opml_string(GString *out, char *str) {
	while (*str != '\0') {
		switch (*str) {
			case '&':
				g_string_append_printf(out, "&amp;");
				break;
			case '<':
				g_string_append_printf(out, "&lt;");
				break;
			case '>':
				g_string_append_printf(out, "&gt;");
				break;
			case '"':
				g_string_append_printf(out, "&quot;");
				break;
			case '\n': case '\r':
				g_string_append_printf(out, "&#10;");
				break;
			default:
				g_string_append_c(out, *str);
		}
		str++;
	}
}
