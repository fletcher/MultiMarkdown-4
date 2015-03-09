/* ensure we only load this once */

#ifndef PARSER_LIB_H
#define PARSER_LIB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include "glib.h"
#include "libMultiMarkdown.h"

#define TABSTOP 4

#define MMD_VERSION "4.7.1"

#define MMD_COPYRIGHT \
	"Copyright (c) 2013-2015 Fletcher T. Penney.\n\n" \
	"LyX export code (c) 2013-2014 Charles R. Cowan,\n" \
	"licensed under both GPL and MIT licenses.\n\n" \
	"portions based on peg-markdown - Copyright (c) 2008-2009 John MacFarlane.\n" \
	"peg-markdown is Licensed under either the GPLv2+ or MIT.\n" \
	"portions Copyright (c) 2011 Daniel Jalkut, MIT licensed.\n\n" \
	"This is free software: you are free to change and redistribute it.\n" \
	"There is NO WARRANTY, to the extent permitted by law.\n\n"


#define DEBUG_OFF		/* Turn on debugging statements (there's a bunch!)*/


/* This is the type used for the $$ pseudovariable passed to parents */
#define YYSTYPE node *

/* This is the data we store in the parser context */
typedef struct {
	const char *charbuf;        /* Input buffer */
	const char *original;       /* Original input buffer */
	node *result;               /* Resulting parse tree */
	unsigned long extensions;   /* Extension bitfield */
	node *autolabels;           /* Store for later retrieval */
	bool  parse_aborted;        /* We got bogged down - fail parse */
	clock_t stop_time;          /* Note the deadline to complete parsing */
} parser_data;

/* A "scratch pad" for storing data when writing output 
	The structure will vary based on what you need */
typedef struct {
	unsigned long extensions;    /* Store copy of extensions for retrieval */
	int   padded;                /* Track newlines */
	int   baseheaderlevel;       /* Increase header levels when outputting */
	int   language;              /* For smart quotes */
	char *table_alignment;       /* Hold the alignment string while parsing table */
	int   table_column;          /* Track the current column number */
	char  cell_type;             /* What sort of cell type are we in? */
	bool  printing_notes;        /* Are we printing notes/glossary/etc.? */
	node *notes;                 /* Store reference notes */
	node *links;                 /* ... links */
	node *glossary;              /* ... glossary */
	node *citations;             /* ... citations */
	node *abbreviations;         /* ... abbreviations */
	node *used_notes;            /* notes that have been referenced */
	node *result_tree;           /* reference to entire result tree */
	int   footnote_to_print;     /* set while we are printing so we can reverse link */
	int   footnote_para_counter; /* so we know which para is last */
	int   max_footnote_num;      /* so we know if current note is new or repeat */
	bool  obfuscate;             /* flag that we need to mask email addresses */
	char *latex_footer;          /* store for appending at the end */
	bool  no_latex_footnote;     /* can't use footnotes in some places */
	int   odf_para_type;         /* what type of paragraph do we need? */
	bool  odf_list_needs_end_p;  /* is there a <p> that need to be closed */
	int   random_seed_base;      /* Allow random footnotes */
	int   toc_level;             /* Track depth for TOC */
	int   table_row;             /* CRC - Track the current row number */
	int   lyx_para_type;         /* CRC - the type of paragraph being processed */
	int   lyx_level;             /* CRC - nesting level */
	bool  no_lyx_footnote;       /* CRC - Can't use footnotes in some places */
	bool  lyx_number_headers;    /* CRC - Whether to number headers (with or without *) */
	bool  lyx_definition_hit;    /* CRC  - True when a definition has been encountered */
	bool  lyx_definition_open;   /* CRC - Have not completed a definition list entry */
	bool  lyx_in_header;         /* CRC - In a table header */
	bool  lyx_in_frame;           /* CRC - in a beamer frame */
	bool  lyx_beamerbullet;      /* CRC - beamer bullet list (add <+->) */
	int   lyx_debug_nest;        /* CRC - nesting level for enhanced debugging */
	bool  lyx_table_need_line;   /* CRC - need a line at the top */
	int   lyx_table_total_rows;  /* CRC - The total number of rows in the table */
	int   lyx_table_total_cols;  /* CRC - The total number of columns in the table */
	node *lyx_table_caption;     /* CRC - Hold the table caption */
	GString *lyx_debug_pad;      /* CRC - padding to indent debugging informaiton */
} scratch_pad;

/* Define smart typography languages -- first in list is default */
enum language {
	ENGLISH,
	DUTCH,
	FRENCH,
	GERMAN,
	GERMANGUILL,
	SWEDISH,
};

/* Character types for smart typography */
enum smartelements {
	LSQUOTE,
	RSQUOTE,
	LDQUOTE,
	RDQUOTE,
	NDASH,
	MDASH,
	ELLIP,
	APOS,
};



/* parser utilities declarations */
node * mk_node(int key);
node * mk_str(char *string);
node * mk_list(int key, node *list);
node * mk_link(node *text, char *label, char *source, char *title, node *attr);
node * mk_pos_node(int key, char *string, unsigned int start, unsigned int stop);
node * mk_pos_str(char *string, unsigned int start, unsigned int stop);
node * mk_pos_list(int key, node *list, unsigned int start, unsigned int stop);

void   free_node(node *n);
void   free_node_tree(node * n);
void   print_node_tree(node * n);
node * copy_node(node *n);
node * copy_node_tree(node *n);

node * cons(node *new, node *list);
node * reverse_list(node *list);
void   append_list(node *new, node *list);

node    * mk_str_from_list(node *list, bool extra_newline);
GString * concat_string_list(node *list);

parser_data * mk_parser_data(const char *charbuf, unsigned long extensions);
void   free_parser_data(parser_data *data);

char * preformat_text(const char *text);

scratch_pad * mk_scratch_pad(unsigned long extensions);
void   free_scratch_pad(scratch_pad *scratch);

link_data * mk_link_data(char *label, char *source, char *title, node *attr);
void   free_link_data(link_data *l);
link_data * extract_link_data(char *label, scratch_pad *scratch);
node * mk_autolink(char *text);

void   extract_references(node *list, scratch_pad *scratch);
void   extract_abbreviations(node *list, scratch_pad *scratch);

bool   extension(int ext, unsigned long extensions);

/* export utilities */
void   trim_trailing_whitespace(char *str);
void   trim_trailing_newlines(char *str);

/* other utilities */
char * label_from_string(char *str);
char * ascii_label_from_string(char *str);
char * clean_string(char *str);
char * string_from_node_tree(node *n);
char * label_from_node_tree(node *n);
char * label_from_node(node *n);
char * ascii_label_from_node_tree(node *n);
char * ascii_label_from_node(node *n);
void   print_raw_node(GString *out, node *n);
void   print_raw_node_tree(GString *out, node*n);

char * correct_dimension_units(char *original);
char * metadata_keys(node *list);
node * metadata_for_key(char *key, node *list);
char * metavalue_for_key(char *key, node *list);

bool tree_contains_key(node *list, int key);
int tree_contains_key_count(node *list, int key);

bool check_timeout();

void debug_node(node *n);
void debug_node_tree(node *n);

char * my_strndup(const char * source, size_t n);

#endif
