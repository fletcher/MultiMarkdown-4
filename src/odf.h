#ifndef ODF_PARSER_H
#define ODF_PARSER_H

#include "parser.h"
#include "html.h"		/* Use some of the HTML routines */
#include "writer.h"

void begin_odf_output(GString *out, node* list, scratch_pad *scratch);
void print_odf_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_odf_node(GString *out, node *n, scratch_pad *scratch);
void print_odf_section_and_children(GString *out, node *list, scratch_pad *scratch);
void end_odf_output(GString *out, node* list, scratch_pad *scratch);
void print_odf_string(GString *out, char *str);
void print_odf_code_string(GString *out, char *str);
void print_odf_header(GString *out);
void print_odf_footer(GString *out);

#endif
