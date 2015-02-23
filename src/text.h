#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include "parser.h"
#include "writer.h"

void print_text_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_text_node(GString *out, node *n, scratch_pad *scratch);


#endif
