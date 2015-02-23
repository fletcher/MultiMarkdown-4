#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include "parser.h"
#include "writer.h"


void print_html_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_html_node(GString *out, node *n, scratch_pad *scratch);
void print_html_localized_typography(GString *out, int character, scratch_pad *scratch);
void print_html_string(GString *out, char *str, scratch_pad *scratch);
void print_html_endnotes(GString *out, scratch_pad *scratch);

#endif
