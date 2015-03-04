#include "parser.h"

#include "text.h"
#include "html.h"
#include "latex.h"
#include "memoir.h"
#include "beamer.h"
#include "lyx.h"
#include "lyxbeamer.h"
#include "opml.h"
#include "odf.h"
#include "rtf.h"
#include "critic.h"
#include "toc.h"

char * export_node_tree(node *list, int format, unsigned long extensions);

void extract_references(node *list, scratch_pad *scratch);
void extract_abbreviations(node *list, scratch_pad *scratch);
void find_abbreviations(node *list, scratch_pad *scratch);

link_data * extract_link_data(char *label, scratch_pad *scratch);

void pad(GString *out, int num, scratch_pad *scratch);

int note_number_for_label(char *text, scratch_pad *scratch);
int note_number_for_node(node *ref, scratch_pad *scratch);
node * node_matching_label(char *label, node *n);
int count_node_from_end(node *n);
int cite_count_node_from_end(node *n);
node * node_for_count(node *n, int count);
void move_note_to_used(node *list, scratch_pad *scratch);
void use_inline_footnote(node *ref, scratch_pad *scratch);
node * node_for_attribute(char *querystring, node *list);

char * dimension_for_attribute(char *querystring, node *list);

link_data * load_link_data(node *n, scratch_pad *scratch);
