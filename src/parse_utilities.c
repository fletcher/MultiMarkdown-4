/*

	parse_utilities.c -- miscellaneous support functions

	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	Derived from peg-multimarkdown, which was forked from peg-markdown,
	which is (c) 2008 John MacFarlane (jgm at berkeley dot edu), and 
	licensed under GNU GPL or MIT.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "parser.h"
#include <libgen.h>

#pragma mark - Parse Tree

/* Create a new node in the parse tree */
node * mk_node(int key) {
	node *result = (node *) malloc(sizeof(node));
	result->key = key;
	result->str = NULL;
	result->children = NULL;
	result->next = NULL;
	result->link_data = NULL;
	return result;
}

/* Create a new node in the parse tree, and store str */
node * mk_str(char *string) {
	node *result = mk_node(STR);
	assert(string != NULL);
	result->str = strdup(string);
	return result;
}

/* mk_str_from_list - merge list into a STR */
node * mk_str_from_list(node *list, bool extra_newline) {
	node *result = mk_node(STR);
	node *rev = reverse_list(list);
	
	GString *c = concat_string_list(rev);
	if (extra_newline)
		g_string_append(c, "\n");

	result->str = c->str;
	
	g_string_free(c, false);
	return result;
}

node * mk_link(node *text, char *label, char *source, char *title, node *attr) {
	node *result = mk_node(LINK);
	result->link_data = mk_link_data(label, source, title, attr);
	result->children = text;
	
	return result;
}

node * mk_autolink(char *text) {
	char *label = label_from_string(text);
	GString *anchor = g_string_new(label);
	g_string_prepend(anchor, "#");

	node *result = mk_node(LINK);
	result->link_data = mk_link_data(label, anchor->str, NULL, NULL);

	g_string_free(anchor, TRUE);
	free(label);
	return result;
}

/* concat_string_list - create string from STR's */
GString * concat_string_list(node *list) {
	GString *result;
	node *next;
	result = g_string_new("");
	while (list != NULL) {
		assert(list->key == STR);
		assert(list->str != NULL);
		g_string_append(result, list->str);
		next = list->next;
		free_node(list);
		list = next;
	}
	return result;
}

/* Create a node that is basically a parent for other elements */
node * mk_list(int key, node *list) {
	node *result;
	result = mk_node(key);
	result->children = reverse_list(list);
	return result;
}
	
/* Create a new node with position information */
node * mk_pos_node(int key, char *string, unsigned int start, unsigned int stop) {
	node *result = mk_node(key);
	if (string != NULL)
		result->str = strdup(string);
	
	return result;
}

/* Create a new string node with position information */
node * mk_pos_str(char *string, unsigned int start, unsigned int stop) {
	node *result = mk_str(string);
	
	return result;
}

/* Create a new list node with position information */
node * mk_pos_list(int key, node *list, unsigned int start, unsigned int stop) {
	node *result = mk_list(key, list);
	
	return result;
}

/* free just the current node and children*/
void free_node(node *n) {
	if (n == NULL)
		return;
	
	if (n->str != NULL)
		free(n->str);
	n->str = NULL;

	free_link_data(n->link_data);
	n->link_data = NULL;
	
	if (n->children != NULL) {
		free_node_tree(n->children);
		n->children = NULL;
	}
	n->next = NULL;
	free(n);
}

/* free element and it's descendents/siblings */
void free_node_tree(node *n) {
	node *next = NULL;
	
	while (n != NULL) {
		next = n->next;
		free_node(n);
		n = next;
	}
}

/* print element list structure for testing */
void print_node_tree(node * n) {
	while (n != NULL) {
		fprintf(stderr,"node key: %d\n",n->key);
		if (n->str != NULL)
			fprintf(stderr,"node str: '%s'\n\n",n->str);
		
		if (n->children != NULL) {
			print_node_tree(n->children);
		}
		n = n->next;
	}
}

/* cons -- add element to list (it goes to the beginning for performance reasons) */
node * cons(node *new, node *list) {
	if (new != NULL) {
		new->next = list;
		return new;
	}
	return list;
}

/* reverse -- reverse a list to get it back into proper order */
node * reverse_list(node *list) {
	node *new = NULL;
	node *next = NULL;
#ifdef DEBUG_ON
	if ((list != NULL) && (list->str != NULL))
	fprintf(stderr, "reverse_list: '%s'\n",list->str);
#endif
	
	while (list != NULL) {
		next = list->next;
		list->next = NULL;
		new = cons(list, new);
		list = next;
	}
	return new;
}

/* append_list -- add element to end of list; slower than cons */
void append_list(node *new, node *list) {

    /* If we append to an empty list... */
    if (list == NULL) {
        list = new;
        return;
    }
    
    if (new != NULL) {
		node *step = list;
        
		
		while (step->next != NULL) {
			step = step->next;
		}
		/*		new->next = NULL; // This could lose elements and prevents merging lists */
		step->next = new;
	}
}

#pragma mark - Parser Data

/* Create parser data - this is where you stash stuff to communicate 
	into and out of the parser */
parser_data * mk_parser_data(const char *charbuf, unsigned long extensions) {
	clock_t start = clock();

	parser_data *result = (parser_data *)malloc(sizeof(parser_data));
	result->extensions = extensions;
	result->charbuf    = charbuf;
	result->original   = charbuf;
	result->autolabels = NULL;
	result->result     = NULL;
	
	result->parse_aborted = 0;
	result->stop_time = start + 3 * CLOCKS_PER_SEC;	/* 3 second timeout */
	
	return result;
}

void free_parser_data(parser_data *data) {
	free_node_tree(data->result);
	free_node_tree(data->autolabels);
/* don't do this - it's owned by someone else -- free(data->original); */
	data->original = NULL;
	data->charbuf = NULL;
	
	free(data);
}

/* mk_scratch_pad -- store stuff here while exporting the result tree */
void ran_start(long seed);
scratch_pad * mk_scratch_pad(unsigned long extensions) {
	scratch_pad *result = (scratch_pad *)malloc(sizeof(scratch_pad));
	result->extensions = extensions;
	result->language = 0;
	result->baseheaderlevel = 1;
	result->printing_notes = 0;
	result->notes       = mk_node(KEY_COUNTER);		/* Need empty need for trimming later */
	result->used_notes  = mk_node(KEY_COUNTER);
	result->links       = mk_node(KEY_COUNTER);
	result->glossary    = mk_node(KEY_COUNTER);
	result->citations   = mk_node(KEY_COUNTER);
	result->abbreviations = mk_node(KEY_COUNTER);
	result->result_tree = NULL;
	result->padded      = 2;
	result->footnote_to_print = 0;
	result->footnote_para_counter = 0;
	result->max_footnote_num = 0;
	result->obfuscate  = 0;
	result->no_latex_footnote = 0;
	result->latex_footer = NULL;
	result->odf_list_needs_end_p = FALSE;
	result->odf_para_type = PARA;
	result->cell_type = 0;
	result->table_alignment = NULL;
	result->table_column = 0;

	if (extensions & EXT_RANDOM_FOOT) {
	    srand((int)time(NULL));
		result->random_seed_base = rand() % 32000;
	} else {
		srand(1);
		result->random_seed_base = 0;
	}
	ran_start(310952L);
	
	result->lyx_para_type = PARA;             /* CRC - Simple paragraph */
	result->lyx_level = 0;                    /* CRC - out outside level */
	result->no_lyx_footnote = 0;              /* CRC  */
	result->lyx_number_headers = FALSE;       /* CRC - default is not to number */
	result->lyx_debug_nest = 0;               /* CRC - initialize debug formatting */
	result->lyx_debug_pad = g_string_new(""); /* CRC - initally, no indent */
	result->lyx_definition_hit = TRUE;        /* CRC - initialize to have hit it (closed) */
	result->lyx_definition_open = FALSE;      /* CRC - don't have an open definition */
	result->lyx_in_frame = FALSE;             /* CRC - not in a frame */
	result->lyx_beamerbullet = FALSE;         /* CRC - not in a beamer bullet */
	result->lyx_debug_nest = 0;               /* CRC - no nesting yet */
	result->lyx_table_need_line = FALSE;      /* CRC - No table yet */
	result->lyx_table_total_rows = 0;         /* CRC - No rows */
	result->lyx_table_total_cols = 0;         /* CRC - No Columns */
	return result;
}

void free_scratch_pad(scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "free scratch pad\n");
#endif
	
	free_node_tree(scratch->notes);
	free_node_tree(scratch->used_notes);
	free_node_tree(scratch->links);
	free_node_tree(scratch->glossary);
	free_node_tree(scratch->citations);
	free_node_tree(scratch->abbreviations);
	
	g_string_free(scratch->lyx_debug_pad, true);    /* CRC - initally, no indent */
	
	if (scratch->latex_footer != NULL)
		free(scratch->latex_footer);
	
	if (scratch->table_alignment != NULL)
		free(scratch->table_alignment);

	free (scratch);
#ifdef DEBUG_ON
	fprintf(stderr, "finished freeing scratch\n");
#endif
}

link_data * mk_link_data(char *label, char *source, char *title, node *attr) {
	link_data *result = (link_data *)malloc(sizeof(link_data));
	if (label != NULL)
		result->label = strdup(label);
	else result->label = NULL;
	if (source != NULL)
		result->source = strdup(source);
	else result->source = NULL;
	if (title != NULL)
		result->title = strdup(title);
	else result->title = NULL;
	
	result->attr = attr;
	
	return result;
}

void free_link_data(link_data *l) {
	if (l == NULL)
		return;
	
	free(l->label);
	l->label = NULL;
	free(l->source);
	l->source = NULL;
	free(l->title);
	l->title = NULL;
	free_node_tree(l->attr);
	l->attr = NULL;
	
	free(l);
}

/* Check if the specified extension is flagged */
bool extension(int ext, unsigned long extensions) {
	return (extensions & ext);
}

/* label_from_string -- convert raw string into format suitable for use as label */
/* As of HTML 5, any character is valid, but no space.  Must be HTML escaped.
   But since we also use in LaTeX, ODF, will be a bit more strict.
   It *looks* like UTF-8 characters are ok in ODF, and don't break LaTeX any more than
   having those characters in the original string.  Will still limit the ASCII characters
   however, to avoid trouble.

   NOTE: This is still a bit experimental, and will be removed if it breaks things.
*/
char *label_from_string(char *str) {
	GString *out = g_string_new("");
	char *label;
	char *next_char;

	while (*str != '\0') {
		next_char = str;
		next_char++;
		/* Is this a multibyte character? */
		if ((*next_char & 0xC0) == 0x80) {
			g_string_append_c(out, *str);
			while ((*next_char & 0xC0) == 0x80) {
				str++;
				/* fprintf(stderr, "multibyte\n"); */
				g_string_append_c(out, *str);
				next_char++;
			}
		}
		
		/* can relax on following characters */
		else if ((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'Z')
			|| (*str >= 'a' && *str <= 'z') || (*str == '.') || (*str== '_')
			|| (*str== '-') || (*str== ':'))
		{
			g_string_append_c(out, tolower(*str));
		}           
		str++;
	}
	label = out->str;
	g_string_free(out, false);
	return label;
}

char *ascii_label_from_string(char *str) {
	GString *out = g_string_new("");
	char *label;
	char *next_char;

	while (*str != '\0') {
		next_char = str;
		next_char++;
		/* Is this a multibyte character? */
		if ((*next_char & 0xC0) == 0x80) {
			while ((*next_char & 0xC0) == 0x80) {
				str++;
				/* fprintf(stderr, "multibyte\n"); */
				next_char++;
			}
		}
		
		/* can relax on following characters */
		else if ((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'Z')
			|| (*str >= 'a' && *str <= 'z') || (*str == '.') || (*str== '_')
			|| (*str== '-') || (*str== ':'))
		{
			g_string_append_c(out, tolower(*str));
		}           
		str++;
	}
	label = out->str;
	g_string_free(out, false);
	return label;
}

/* clean_string -- clean up whitespace */
char * clean_string(char *str) {
	GString *out = g_string_new("");
	char *clean;
	bool block_whitespace = TRUE;
	
	while (*str != '\0') {
		if ((*str == '\t') || (*str == ' ') || (*str == '\n') || (*str == '\r')) {
			if (!block_whitespace) {
				g_string_append_c(out, ' ');
				block_whitespace = TRUE;
			}
		} else {
			g_string_append_c(out, *str);
			block_whitespace = FALSE;
		}
		str++;
	}
	
	clean = out->str;
	g_string_free(out, false);
	return clean;
}

/* string_from_node_tree -- Returns a null-terminated string,
	which must be freed after use. */
char * string_from_node_tree(node *n) {
	char *result;
	if (n == NULL)
		return NULL;

	GString *raw = g_string_new("");
	print_raw_node_tree(raw, n);

	result = raw->str;
	g_string_free(raw, false);

	return result;
}

/* label_from_node_tree -- Returns a null-terminated string,
	which must be freed after use. */
char * label_from_node_tree(node *n) {
	char *label;
	if (n == NULL)
		return NULL;
	
#ifdef DEBUG_ON
		fprintf(stderr, "\n\nstart label from node_tree\n");
#endif
	GString *raw = g_string_new("");
	print_raw_node_tree(raw, n);

#ifdef DEBUG_ON
		fprintf(stderr, "halfway('%s')\n",raw->str);
#endif
	label =  label_from_string(raw->str);
	g_string_free(raw,true);
#ifdef DEBUG_ON
		fprintf(stderr, "finish label from node_tree: '%s'\n",label);
#endif
	return label;
}

/* ascii_label_from_node_tree -- Returns a null-terminated string,
	which must be freed after use. */
char * ascii_label_from_node_tree(node *n) {
	char *label;
	if (n == NULL)
		return NULL;
	
#ifdef DEBUG_ON
		fprintf(stderr, "\n\nstart label from node_tree\n");
#endif
	GString *raw = g_string_new("");
	print_raw_node_tree(raw, n);

#ifdef DEBUG_ON
		fprintf(stderr, "halfway('%s')\n",raw->str);
#endif
	label =  ascii_label_from_string(raw->str);
	g_string_free(raw,true);
#ifdef DEBUG_ON
		fprintf(stderr, "finish label from node_tree: '%s'\n",label);
#endif
	return label;
}

/* label_from_node -- Returns a null-terminated string,
	which must be freed after use. */
char *label_from_node(node *n) {
	char *label;
	char *label2;
	if (n == NULL)
		return NULL;
	
	GString *raw = g_string_new("");
	print_raw_node(raw, n);
	label =  label_from_string(raw->str);
	label2 = strdup(label);
	free(label);
	g_string_free(raw,true);
	return label2;
}

/* ascii_label_from_node -- Returns a null-terminated string,
	which must be freed after use. */
char *ascii_label_from_node(node *n) {
	char *label;
	char *label2;
	if (n == NULL)
		return NULL;
	
	GString *raw = g_string_new("");
	print_raw_node(raw, n);
	label =  ascii_label_from_string(raw->str);
	label2 = strdup(label);
	free(label);
	g_string_free(raw,true);
	return label2;
}

/* print_raw_node - print an element as original text */
void print_raw_node(GString *out, node *n) {
	if (n->str != NULL) {
#ifdef DEBUG_ON
		fprintf(stderr, "print raw node %d: '%s'\n",n->key, n->str);
#endif
		g_string_append_printf(out, "%s", n->str);
	} else if (n->key == LINK) {
#ifdef DEBUG_ON
		fprintf(stderr, "print raw node children from link\n");
#endif
		/* this gets the text */
		print_raw_node_tree(out, n->children);
		/* need the label */
		if ((n->link_data != NULL) && (n->link_data->label != NULL))
			g_string_append_printf(out, "%s",n->link_data->label);
	} else {
		/* All others */
#ifdef DEBUG_ON
		fprintf(stderr, "print raw node children from %d\n",n->key);
#endif
		print_raw_node_tree(out, n->children);
	}
#ifdef DEBUG_ON
		fprintf(stderr, "finish print raw node %d: '%s'\n'%s'\n",n->key, n->str, out->str);
#endif
}

/* print_raw_node_tree - print a list of elements as original text */
void print_raw_node_tree(GString *out, node *n) {
#ifdef DEBUG_ON
	if (n != NULL)
		fprintf(stderr, "start print_raw_node_tree: '%d'\n",n->key);
#endif
	while (n != NULL) {
		print_raw_node(out, n);
		n = n->next;
	}
#ifdef DEBUG_ON
	if (n != NULL)
		fprintf(stderr, "finish print_raw_node_tree: '%d'\n",n->key);
#endif
}

/* preformat_text - allocate and copy text buffer while
 * performing tab expansion. */
char * preformat_text(const char *text) {
	GString *buf;
	char next_char;
	int charstotab;
	char *out;

	int len = 0;

	buf = g_string_new("");

	charstotab = TABSTOP;
	while ((next_char = *text++) != '\0') {
		switch (next_char) {
			case '\t':
				while (charstotab > 0)
					g_string_append_c(buf, ' '), len++, charstotab--;
				break;
			case '\n':
				g_string_append_c(buf, '\n'), len++, charstotab = TABSTOP;
				break;
			default:
				g_string_append_c(buf, next_char), len++, charstotab--;
		}
		if (charstotab == 0)
			charstotab = TABSTOP;
	}
	g_string_append_printf(buf, "\n\n");
	out = buf->str;
	g_string_free(buf,false);
	return(out);
}

/* Don't let us get caught in "infinite" loop;
	1 means we're ok 
	0 means we're stuck -- abort */
bool check_timeout(parser_data *data) {
	/* Once we abort, keep aborting */
	if (data->parse_aborted)
		return 0;
	if (clock() > data->stop_time) {
		data->parse_aborted = 1;
		return 0;
	}
	return 1;
}

/* determine whether a certain element is contained within a given list */
bool tree_contains_key(node *list, int key) {
	node *step = NULL;

	step = list;
	while ( step != NULL ) {
		if (step->key == key) {
			return TRUE;
		}
		if (step->children != NULL) {
			if (tree_contains_key(step->children, key)) {
				return TRUE;
			}
		}
		step = step->next;
	}
	return FALSE;
}

/* Count number of matches of type */
int tree_contains_key_count(node *list, int key) {
	node *step = NULL;
	int counter = 0;

	step = list;
	while ( step != NULL ) {
		if (step->key == key) {
			counter++;
		}
		if (step->children != NULL) {
			counter += tree_contains_key_count(step->children, key);
		}
		step = step->next;
	}
	return counter;
}

/* list all metadata keys, if present */
char * metadata_keys(node *list) {
	node *step = NULL;
	step = list;
	GString *out = g_string_new("");
	char *temp;
		
	while (step != NULL) {
		if (step->key == METADATA) {
			/* search METAKEY children */
			step = step->children;
			while ( step != NULL) {
				temp = label_from_string(step->str);
				g_string_append_printf(out,"%s\n",temp);
				free(temp);
				step = step->next;
			}
			
			temp = out->str;
			g_string_free(out, false);

			return temp;
		}
		step = step->next;
	}
	temp = out->str;
	g_string_free(out, false);

	return temp;
}

/* find specified metadata key, if present */
node * metadata_for_key(char *key, node *list) {
	node *step = NULL;
	step = list;
	char *label;
	char *temp;
	
	label = label_from_string(key);
	
	while (step != NULL) {
		if (step->key == METADATA) {
			/* search METAKEY children */
			step = step->children;
			while ( step != NULL) {
				temp = label_from_string(step->str);
				if (strcmp(temp, label) == 0) {
					free(temp);
					free(label);
					return step;
				}
				free(temp);
				step = step->next;
			}
			free(label);
			return NULL;
		}
		step = step->next;
	}
	free(label);
	return NULL;
}

/* metavalue_for_key -- return the string value for metadata */
char * metavalue_for_key(char *key, node *list) {
	char *result;
	
	list = metadata_for_key(key, list);
	
	if (list == NULL)
		return NULL;
	
	result = strdup(list->children->str);
	trim_trailing_whitespace(result);
	
	return result;
}

/* Trim spaces at end of string */
void trim_trailing_whitespace(char *str) {
	unsigned long l;
	
	if (str == NULL)
		return;
	
	l = strlen(str);
	
	if (l < 1)
		return;
	
	while ( (l > 0) && (( str[l - 1] == ' ' ) ||
		( str[l - 1] == '\n' ) || 
		( str[l - 1] == '\r' ) || 
		( str[l - 1] == '\t' )) ) {
		str[l - 1] = '\0';
		l = strlen(str);
	}
}

/* Trim spaces at end of string */
void trim_trailing_newlines(char *str) {
	unsigned long l;
	
	if (str == NULL)
		return;
	
	l = strlen(str);
	
	if (l < 1)
		return;
	
	while ( (l > 0) && (( str[l - 1] == '\n' ) || 
	( str[l - 1] == '\r' )) ) {
		str[l - 1] = '\0';
		l = strlen(str);
	}
}

/* Return version */
char * mmd_version(void) {
	char *result;
	result = strdup(MMD_VERSION);
	return result;
}

void debug_node(node *n) {
	if (n != NULL) {
		fprintf(stderr, "node (%d) '%s'\n",n->key, n->str);
		if (n->children != NULL)
			debug_node_tree(n->children);
	}
}

void debug_node_tree(node *n) {
	while (n != NULL) {
		fprintf(stderr, "node (%d) '%s'\n",n->key, n->str);
		if (n->children != NULL)
			debug_node_tree(n->children);
		n = n->next;
	}
}

node * copy_node(node *n) {
	if (n == NULL)
		return NULL;
	else {
		node *m = (node *) malloc(sizeof(node));

		*m = *n;

		if (n->str != NULL)
			m->str = strdup(n->str);

		if (n->link_data != NULL) {
			m->link_data = mk_link_data(n->link_data->label, n->link_data->source, n->link_data->title, copy_node_tree(n->link_data->attr));
		}

		if (n->children != NULL)
			m->children = copy_node_tree(n->children);

		return m;
	}
}

node * copy_node_tree(node *n) {
	if (n == NULL)
		return NULL;
	else {
		node *m = copy_node(n);
		
		if (n->next != NULL)
			m->next = copy_node_tree(n->next);
	
		return m;
	}
}

char * my_strndup(const char * source, size_t n) {
	size_t len = strlen(source);
	char * result;

	if (n < len)
		len = n;

	result = malloc(len + 1);

	memcpy(result, source, len);
	result[len] = '\0';

	return result;
}