#ifndef TOC_PARSER_H
#define TOC_PARSER_H

#include "parser.h"
#include "writer.h"

void begin_toc_output(GString *out, node* list, scratch_pad *scratch);
void print_toc_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_toc_node(GString *out, node *n, scratch_pad *scratch);
void print_toc_section_and_children(GString *out, node *list, scratch_pad *scratch);
void end_toc_output(GString *out, node* list, scratch_pad *scratch);
void print_toc_string(GString *out, char *str);


#endif
