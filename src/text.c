/*

	test.c -- plain text writer function as an example.
		Recreates the input source.

	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.


	WARNING -- Not complete and not intended for use

*/

#include "text.h"


/* print_text_node_tree -- convert node tree to plain text */
void print_text_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_text_node(out, list, scratch);
		list = list->next;
	}
}

/* print_text_node -- convert given node to plain text and append */
void print_text_node(GString *out, node *n, scratch_pad *scratch) {
	switch (n->key) {
		case STR:
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
		case FOOTER:
			break;
		default:
			fprintf(stderr, "print_text_node encountered unknown node key = %d\n",n->key);
			exit(EXIT_FAILURE);
	}
}
