#ifndef MEMOIR_PARSER_H
#define MEMOIR_PARSER_H

#include "parser.h"
#include "latex.h"

void print_memoir_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_memoir_node(GString *out, node *n, scratch_pad *scratch);

#endif
