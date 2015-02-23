#ifndef CRITIC_PARSER_H
#define CRITIC_PARSER_H

#include "parser.h"
#include "writer.h"


void print_critic_accept_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_critic_reject_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_critic_html_highlight_node_tree(GString *out, node *list, scratch_pad *scratch);
void print_critic_accept_node(GString *out, node *list, scratch_pad *scratch);
void print_critic_reject_node(GString *out, node *list, scratch_pad *scratch);
void print_critic_html_highlight_node(GString *out, node *list, scratch_pad *scratch);

#endif
