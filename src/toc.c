/*

	toc.c -- Table of contents

	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "toc.h"


/* print_toc_node_tree -- convert node tree to MultiMarkdown */
void print_toc_node_tree(GString *out, node *list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "print_toc_node_tree\n");
#endif
	int lev;
	while (list != NULL) {
		if (list->key == HEADINGSECTION) {
			lev = list->children->key;
			
			print_toc_section_and_children(out, list, scratch);
			
			while ((list->next != NULL) && (list->next->key == HEADINGSECTION)
				&& (list->next->children->key > lev)) {
					list = list->next;
				}
		} else {
			print_toc_node(out, list, scratch);
		}
		list = list->next;
	}
}

/* print_toc_section_and_children -- we want to stay inside the outline structure */
void print_toc_section_and_children(GString *out, node *list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "print_toc_section_and_children: %d\n",list->key);
#endif
	int lev = list->children->key;

	/* print current section (parent) */
	print_toc_node(out, list, scratch);

	scratch->toc_level ++;
	
	/* check for child nodes */
	while ((list->next != NULL) && (list->next->key == HEADINGSECTION) && (list->next->children->key > lev)) {
		/* next item is also a HEADINGSECTION and is a child */
		if (list->next->children->key - lev == 1)
			print_toc_section_and_children(out, list->next, scratch);
		list = list->next;
	}

	scratch->toc_level --;
}

/* print_toc_node -- convert given node to MultiMarkdown and append */
void print_toc_node(GString *out, node *n, scratch_pad *scratch) {
	char *temp;
	int i;

#ifdef DEBUG_ON
	fprintf(stderr, "print_toc_node: %d\n",n->key);
#endif
	switch (n->key) {
		case HEADINGSECTION:
			/* Need to handle "nesting" properly */
			for (i = 0; i < scratch->toc_level; ++i)
			{
				g_string_append_printf(out, "\t");
			}
			g_string_append_printf(out, "* ");

			/* Print header */
			print_toc_node(out, n->children, scratch);

			break;
		case H1: case H2: case H3: case H4: case H5: case H6: 
			if ((n->children != NULL) && (n->children->key == AUTOLABEL)) {
				temp = label_from_string(n->children->str);
				/* use label for header since one was specified (MMD)*/
				g_string_append_printf(out, "[");
				print_toc_node_tree(out, n->children, scratch);
				g_string_append_printf(out, "][%s]\n", temp);
			} else {
				temp = label_from_node_tree(n->children);
				g_string_append_printf(out, "[");
				print_toc_node_tree(out, n->children, scratch);
				g_string_append_printf(out, "][%s]\n", temp);
			}
			free(temp);
			break;
		case STR:
			print_toc_string(out, n->str);
			break;
		case EMPH:
			g_string_append_printf(out, "*");
			print_toc_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "*");
			break;
		case STRONG:
			g_string_append_printf(out, "**");
			print_toc_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "**");
			break;
		case SPACE:
			g_string_append_printf(out, "%s", n->str);
			break;
		case LINK:
			print_toc_node_tree(out, n->children, scratch);
			break;
		case HTML:
			break;
		case LINKREFERENCE:
			break;
		case AUTOLABEL:
			break;
		case VARIABLE:
			g_string_append_printf(out, "[%%%s]",n->str);
			break;
		case LIST:
			print_toc_node_tree(out, n->children, scratch);
			break;
		default: fprintf(stderr, "print_toc_node encountered unknown node key = %d\n",n->key);
			break;
	}
#ifdef DEBUG_ON
	fprintf(stderr, "finish print_toc_node: %d\n", n->key);
#endif
}

/* print_toc_string - print string, escaping for MultiMarkdown */
void print_toc_string(GString *out, char *str) {
	while (*str != '\0') {
		switch (*str) {
			case '[':
				g_string_append_printf(out, "\\[");
				break;
			case ']':
				g_string_append_printf(out, "\\]");
				break;
			default:
				g_string_append_c(out, *str);
		}
		str++;
	}
}
