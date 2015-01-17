#ifndef RTF_PARSER_H
#define RTF_PARSER_H

#include "parser.h"
#include "writer.h"

void begin_rtf_output(GString *out, node* list, scratch_pad *scratch);
void end_rtf_output(GString *out, node* list, scratch_pad *scratch);
void print_rtf_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_rtf_node(GString *out, node *n, scratch_pad *scratch);
void print_rtf_localized_typography(GString *out, int character, scratch_pad *scratch);
void print_rtf_string(GString *out, char *str, scratch_pad *scratch);
void print_rtf_code_string(GString *out, char *str, scratch_pad *scratch);
void print_rtf_endnotes(GString *out, scratch_pad *scratch);
void pad_rtf(GString *out, int pad, scratch_pad *scratch);

#endif
