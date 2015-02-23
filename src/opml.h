#ifndef OPML_PARSER_H
#define OPML_PARSER_H

#include "parser.h"
#include "writer.h"

void begin_opml_output(GString *out, node* list, scratch_pad *scratch);
void print_opml_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_opml_node(GString *out, node *n, scratch_pad *scratch);
void print_opml_section_and_children(GString *out, node *list, scratch_pad *scratch);
void end_opml_output(GString *out, node* list, scratch_pad *scratch);
void print_opml_string(GString *out, char *str);


#endif
