#ifndef BEAMER_PARSER_H
#define BEAMER_PARSER_H

#include "parser.h"
#include "latex.h"

void print_beamer_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_beamer_node(GString *out, node *n, scratch_pad *scratch);
void print_beamer_endnotes(GString *out, scratch_pad *scratch);

#endif
