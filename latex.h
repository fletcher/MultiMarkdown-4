#ifndef LATEX_PARSER_H
#define LATEX_PARSER_H

#include "parser.h"
#include "writer.h"


void print_latex_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_latex_node(GString *out, node *n, scratch_pad *scratch);
void print_latex_localized_typography(GString *out, int character, scratch_pad *scratch);
void print_latex_string(GString *out, char *str, scratch_pad *scratch);
void print_latex_url(GString *out, char *str, scratch_pad *scratch);
void print_latex_endnotes(GString *out, scratch_pad *scratch);
int  find_latex_mode(int format, node *n);

#endif
