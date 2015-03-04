/*

	lyx.c -- LyX writer

	(c) 2013 Charles R. Cowan
	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).

	Derived from MultiMarkdown by Fletcher T. Penney - added code to support the LyX format directly
	
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

#include "lyx.h"

/* #define DEBUG_ON */

/* #define DUMP_TREES */

/* allow the user to change the heading levels */

GString *heading_name[7];
GString *used_abbreviations;

#if defined(DEBUG_ON) || defined(DUMP_TREES)
  const char * const node_types[] = {
  "NO_TYPE",
  "LIST",
  "STR",
  "APOSTROPHE",
  "FOOTER",
  "PARA",
  "PLAIN",
  "LINEBREAK",
  "SPACE",
  "HEADINGSECTION",
  "H1", 
  "H2", 
  "H3", 
  "H4", 
  "H5", 
  "H6", 
  "H7", 
  "METADATA",
  "METAKEY",
  "METAVALUE",
  "MATHSPAN",
  "STRONG",
  "EMPH",
  "LINK",
  "SOURCE",
  "TITLE",
  "REFNAME",
  "AUTOLABEL",
  "IMAGE",
  "IMAGEBLOCK",
  "NOTEREFERENCE",
  "CODE",
  "HTML",
  "ELLIPSIS",
  "ENDASH",
  "EMDASH",
  "SINGLEQUOTED",
  "DOUBLEQUOTED",
  "BLOCKQUOTE",
  "BLOCKQUOTEMARKER",
  "RAW",
  "VERBATIM",
  "VERBATIMTYPE",
  "DEFLIST",
  "TERM",
  "DEFINITION",
  "HRULE",
  "ORDEREDLIST",
  "BULLETLIST",
  "LISTITEM",
  "HTMLBLOCK",
  "TABLE",
  "TABLECAPTION",
  "TABLELABEL",
  "TABLESEPARATOR",
  "TABLECELL",
  "CELLSPAN",
  "TABLEROW",
  "TABLEBODY",
  "TABLEHEAD",
  "LINKREFERENCE",
  "NOTESOURCE",
  "CITATIONSOURCE",
  "SOURCEBRANCH",
  "NOTELABEL",
  "GLOSSARYLABEL",
  "ATTRVALUE",
  "ATTRKEY",
  "GLOSSARYSOURCE",
  "GLOSSARYSORTKEY",
  "GLOSSARYTERM",
  "CITATION",
  "NOCITATION",
  "CRITICADDITION",
  "CRITICDELETION",
  "CRITICSUBSTITUTION",
  "CRITICHIGHLIGHT",
  "CRITICCOMMENT",
  "SUPERSCRIPT",
  "SUBSCRIPT",
  "VARIABLE",
  "ABBREVIATION",
  "ABBR",
  "ABBRSTART",
  "ABBRSTOP",
  "KEY_COUNTER"
  }; 
  
  
#endif


#ifdef DUMP_TREES

void dump_tree(node* n, scratch_pad *scratch)
{
	int i;
	while(n != NULL){
	   scratch->lyx_debug_nest++;
       g_string_append(scratch->lyx_debug_pad,"  "); /* add a level */
	   fprintf(stderr, "\n\n%sNode: %s",scratch->lyx_debug_pad->str,node_types[n->key]);
	   fprintf(stderr, "\n%s  str: %s",scratch->lyx_debug_pad->str,n->str);
	   if (n->link_data != NULL){
	   	 fprintf(stderr, "\n%s  label: %s",scratch->lyx_debug_pad->str,n->link_data->label);
	   	 fprintf(stderr, "\n%s  source: %s",scratch->lyx_debug_pad->str,n->link_data->source);
	   	 fprintf(stderr, "\n%s  title: %s",scratch->lyx_debug_pad->str,n->link_data->title);
	   }
	   	  dump_tree(n->children,scratch);
	   scratch->lyx_debug_nest--;
	   g_string_free(scratch->lyx_debug_pad,TRUE); /* don' see a way to shorten the string */
	   scratch->lyx_debug_pad = g_string_new(""); /* so create a new, shorter one */
	   for(i=0;i<scratch->lyx_debug_nest;i++){
	    g_string_append(scratch->lyx_debug_pad,"  ");
       }
			
		n = n->next;
    }
}
#endif 


void perform_lyx_output(GString *out, node* list, scratch_pad *scratch)
{
	node *headings;
	node *base_header_level;
	char *key;

	
#ifdef DUMP_TREES
      fprintf(stderr, "\n*** Base Tree ***\n");
      dump_tree(list,scratch);
      fprintf (stderr, "\n*** End Base Tree ***\n");
      
      fprintf(stderr, "\n*** LINK Tree ***\n");
      dump_tree(scratch->links,scratch);
      fprintf(stderr, "\n*** END LINK Tree ***\n");

#endif
			
	/* initialize the heading names */
	heading_name[0] = g_string_new("Part");
	heading_name[1] = g_string_new("Chapter");
	heading_name[2] = g_string_new("Section");
	heading_name[3] = g_string_new("Subsection");
	heading_name[4] = g_string_new("Subsubsection");
	heading_name[5] = g_string_new("Paragraph");
	heading_name[6] = g_string_new("Subparagraph");
	
		/* get user supplied heading section names and base header level these both
		   affect creating the prefixes */
	
	GString *lyx_headings = g_string_new("");
	used_abbreviations = g_string_new("");
	int hcount;
	hcount = 0;
	const char s[2] = ",";
    char *token;
    char *cleaned;
	if (tree_contains_key(list, METAKEY)) {
		headings = metadata_for_key("lyxheadings",list);
		if (headings != NULL) {
			key = metavalue_for_key("lyxheadings",list);
			g_string_append(lyx_headings,key);
			token = strtok(lyx_headings->str, s);
			 while( token != NULL )  {
			   g_string_free(heading_name[hcount],TRUE);
			   cleaned = clean_string(token);
			   heading_name[hcount] = g_string_new(cleaned);
			   free(cleaned);
               token = strtok(NULL, s);
               hcount++;
			   if (hcount>6){ /* only 7 allowed */
			     break;
				 }
             }
			free(key);
			free(token);
		} 
	}
	g_string_free(lyx_headings,TRUE);
	
	/* get base heading level */
        scratch->baseheaderlevel = 1;
        if (tree_contains_key(list, METAKEY)) {
		base_header_level = metadata_for_key("baseheaderlevel",list);
		if (base_header_level != NULL) {
			key = metavalue_for_key("baseheaderlevel",list);
			scratch->baseheaderlevel = atoi(key);
			free(key);
		};
	};
	
	
	/* add prefixes for LyX references */
	add_prefixes(list, list, scratch);
	
	bool isbeamer;
	isbeamer = begin_lyx_output(out,list,scratch);    /* get Metadata controls */
	if (isbeamer){
		g_string_free(heading_name[1],TRUE);
		heading_name[1] = g_string_new("Section");
		g_string_free(heading_name[2],TRUE);
		heading_name[2] = g_string_new("Frame");
		print_lyxbeamer_node_tree(out,list,scratch,FALSE);
	} else {
	print_lyx_node_tree(out,list,scratch,FALSE);  
	}
	end_lyx_output(out,list,scratch);                 /* Close the document */
	
	/* clean up the heading names */
	int i;
	for (i=0;i<=6;i++){
		g_string_free(heading_name[i],TRUE);   
	}
    g_string_free(used_abbreviations,TRUE);
}

/* begin_lyx_output -- Check metadata and open the document */
bool begin_lyx_output(GString *out, node* list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "begin_lyx_output\n");
#endif
	node *content;
	node *latex_mode;
	node *number_headings;
	node *clean_pdf;
	node *quote_language;
	node *modules;
	node *packages;
	char short_prefix[6];
	int i,j;
	const char s[2] = ",";
	char *key;
	char *label;
	char *value;
	char *temp;
	char *token;
	char *tmp;
	char *cleaned;
	bool isbeamer;  /* beamer has different processing */
	
	isbeamer = FALSE; /* only for beamer */
	

	/* check for numbered versus unnumbered headings */
	    scratch->lyx_number_headers = TRUE; /* default - numbering */
		if (tree_contains_key(list, METAKEY)) {
		number_headings = metadata_for_key("numberheadings",list);
		if (number_headings != NULL) {
			key = metavalue_for_key("numberheadings",list);
			label = label_from_string(key);
			if (strcmp(label, "yes") == 0) {
				scratch->lyx_number_headers = TRUE;
			} else if (strcmp(label, "no") == 0) {
				scratch->lyx_number_headers = FALSE;
			}
			free(label);
			free(key);
		}
	}
	
	/* Get the language for quotes */
		if (tree_contains_key(list, METAKEY)) {
		quote_language = metadata_for_key("quoteslanguage",list);
		if (quote_language != NULL) {
			key = metavalue_for_key("quoteslanguage",list);
			temp = label_from_node_tree(quote_language->children);
			if ((strcmp(temp, "nl") == 0) || (strcmp(temp, "dutch") == 0)) { scratch->language = DUTCH; }   else 
			if ((strcmp(temp, "de") == 0) || (strcmp(temp, "german") == 0)) { scratch->language = GERMAN; } else 
			if (strcmp(temp, "germanguillemets") == 0) { scratch->language = GERMANGUILL; } else 
			if ((strcmp(temp, "fr") == 0) || (strcmp(temp, "french") == 0)) { scratch->language = FRENCH; } else 
			if ((strcmp(temp, "sv") == 0) || (strcmp(temp, "swedish") == 0)) { scratch->language = SWEDISH; }
			free(temp);
			free(key);
			}
	    }
			
	g_string_append(out, "#LyX File created by multimarkdown\n");
	g_string_append(out,"\\lyxformat 413\n");
	g_string_append(out, "\\begin_document\n");
	g_string_append(out, "\\begin_header\n");
	
	GString *lyx_class = g_string_new("");
	if (tree_contains_key(list, METAKEY)) {
		latex_mode = metadata_for_key("latexmode",list);
		if (latex_mode != NULL) {
			key = metavalue_for_key("latexmode",list);
			label = label_from_string(key);
			g_string_append(lyx_class,label);
			if (strcmp(label,"beamer")==0){
				isbeamer = TRUE;
			}
			free(label);
			free(key);
		} else {
			g_string_append(lyx_class,"memoir");
		}
		
	}else{
	     g_string_append(lyx_class,"memoir");	
	}
	g_string_append(out,"\\textclass ");
	g_string_append_printf(out,"%s",lyx_class->str);
	g_string_append(out,"\n");
	
	g_string_append(out,"\\begin_preamble\n");
	g_string_append(out,"\\usepackage{listings}\n");
	g_string_append(out,"\\usepackage{natbib}\n");
	g_string_append(out,"\\usepackage{nomencl}\n");
	g_string_append(out,"\\usepackage{booktabs}\n");
	g_string_append(out,"\\usepackage{refstyle}\n");
	g_string_append(out,"\\usepackage{varioref}\n");
	
	
	if (tree_contains_key(list, METAKEY)) {
		packages = metadata_for_key("packages",list);
		if (packages != NULL) {
			key = metavalue_for_key("packages",list);
			tmp = strdup(key);
			token = strtok(tmp, s);
			 while( token != NULL )  {
               g_string_append_printf(out,"\\usepackage{%s}\n",clean_string(token));
               token = strtok(NULL, s);
             }
			free(key);
			free(tmp);
		} 
	}
	
	if(isbeamer){
	  if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("theme", list);
		if (content != NULL) {
			value = metavalue_for_key("theme",list);
			g_string_append_printf(out,"\\usetheme{%s}\n",value);
			free(value);
		} else{
			g_string_append(out,"\\usetheme{warsaw}\n");
		}
	  } else {
		g_string_append(out,"\\usetheme{warsaw}\n");
	  }	
	  g_string_append(out,"\\setbeamercovered{transparent}\n");
	}
	
	if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("latex input", list);
		if (content != NULL) {
			value = metavalue_for_key("latex input",list);
			if (strcmp(value,"mmd-natbib-plain")==0){
				g_string_append(out,"\\bibpunct{[}{]}{;}{n}{}{,}\n");
			}else{
				g_string_append(out,"\\bibpunct{(}{)}{,}{a}{,}{,}\n");
			}
			free(value);
		} else{
			g_string_append(out,"\\bibpunct{(}{)}{,}{a}{,}{,}\n");
		}
	} else {
		g_string_append(out,"\\bibpunct{(}{)}{,}{a}{,}{,}\n");
	}

    /* set up nice referencing */
    
    if (scratch->lyx_number_headers){
	    for(i=0;i<7;i++){
		  strncpy(short_prefix,heading_name[i]->str,5);
		  short_prefix[5]= '\0'; /* no terminator if strncpy ends because of length */
		  for(j = 0; short_prefix[j]; j++){
	    		short_prefix[j] = tolower(short_prefix[j]);
	      }
	      g_string_append_printf(out,"\\newref{%s}{refcmd={%s \\ref{#1} \\vpageref{#1}}}\n",short_prefix,heading_name[i]->str);
	    }
	    g_string_append(out,"\\newref{tab}{refcmd={Table \\ref{#1} \\vpageref{#1}}}\n");
	    g_string_append(out,"\\newref{fig}{refcmd={Figure \\ref{#1} \\vpageref{#1}}}\n");
    } else {
    	for(i=0;i<7;i++){
		  strncpy(short_prefix,heading_name[i]->str,5);
		  short_prefix[5]= '\0'; /* no terminator if strncpy ends because of length */
		  for(j = 0; short_prefix[j]; j++){
	    		short_prefix[j] = tolower(short_prefix[j]);
	      }
	      g_string_append_printf(out,"\\newref{%s}{refcmd={``\\nameref{#1}'' \\vpageref{#1}}}\n",short_prefix);
	    }
        g_string_append(out,"\\newref{tab}{refcmd={Table \\ref{#1} \\vpageref{#1}}}\n");
	    g_string_append(out,"\\newref{fig}{refcmd={Figure \\ref{#1} \\vpageref{#1}}}\n");
            	
    }

	g_string_append(out,"\\end_preamble\n");
	
	GString *class_options = g_string_new("\\options refpage");
	
	if (tree_contains_key(list, METAKEY)) {
		clean_pdf = metadata_for_key("cleanpdf",list);
		if (clean_pdf != NULL) {
			key = metavalue_for_key("cleanpdf",list);
			label = label_from_string(key);
			if (strcmp(label, "yes") == 0) {
				g_string_append(class_options,",hidelinks");
			} 
			free(label);
			free(key);
		}
	}
	
    if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("class options", list);
		if (content != NULL) {
			value = metavalue_for_key("class options",list);
			g_string_append(class_options,",");
			g_string_append(class_options,value);
			free(value);
		}
	}
	g_string_append(class_options,"\n");
	g_string_append(out,class_options->str);
	g_string_free(class_options,TRUE);
	
			
		
	g_string_append(out,"\\begin_modules\n");
	if (tree_contains_key(list, METAKEY)) {
		modules = metadata_for_key("modules",list);
		if (modules != NULL) {
			key = metavalue_for_key("modules",list);
			tmp = strdup(key);
			token = strtok(tmp, s);
			 while( token != NULL )  {
			   cleaned = clean_string(token);
               g_string_append_printf(out,"%s\n",cleaned);
               free(cleaned);
               token = strtok(NULL, s);
             }
			free(key);
			free(tmp);
			free(token);
		} 
	}
	 g_string_append(out,"\\end_modules\n");
	
	g_string_append(out,"\\bibtex_command default\n");
	g_string_append(out,"\\cite_engine natbib_authoryear\n");
	
	g_string_free(lyx_class,TRUE);
	
	g_string_append(out,"\\end_header\n");
	g_string_append(out,"\\begin_body\n");
	
	if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("title", list);
		if (content != NULL) {
			g_string_append(out, "\n\\begin_layout Title\n");
			value = metavalue_for_key("title",list);
            print_lyx_string(out,value,scratch,LYX_NONE);
			free(value);
			g_string_append(out, "\n\\end_layout\n");
		}
	}
	
	if ((isbeamer) && (tree_contains_key(list, METAKEY))) {
		content = metadata_for_key("subtitle", list);
		if (content != NULL) {
			g_string_append(out, "\n\\begin_layout Subtitle\n");
			value = metavalue_for_key("subtitle",list);
			print_lyx_string(out,value,scratch,LYX_NONE);
			free(value);
			g_string_append(out, "\n\\end_layout\n");
		}
	}
	
	if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("author", list);
		if (content != NULL) {
			g_string_append(out, "\n\\begin_layout Author\n");
			value = metavalue_for_key("author",list);
			print_lyx_string(out,value,scratch,LYX_NONE);
			free(value);
			g_string_append(out, "\n\\end_layout\n");
		}
	}
	
	if ((isbeamer) && (tree_contains_key(list, METAKEY))){
		content = metadata_for_key("affiliation", list);
		if (content != NULL) {
			g_string_append(out, "\n\\begin_layout Institute\n");
			value = metavalue_for_key("affiliation",list);
			print_lyx_string(out,value,scratch,LYX_NONE);
			free(value);
			g_string_append(out, "\n\\end_layout\n");
		}
	}
	
	if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("date", list);
		if (content != NULL) {
			g_string_append(out, "\n\\begin_layout Date\n");
			value = metavalue_for_key("date",list);
			print_lyx_string(out,value,scratch,LYX_NONE);
			free(value);
			g_string_append(out, "\n\\end_layout\n");
		}
	}
	
	if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("abstract", list);
		if (content != NULL) {
			g_string_append(out, "\n\\begin_layout Abstract\n");
			value = metavalue_for_key("abstract",list);
            print_lyx_string(out,value,scratch,LYX_NONE);
			free(value);
			g_string_append(out, "\n\\end_layout\n");
		}
	}
    return isbeamer;	
}

/* end_lyx_output -- close the document */
void end_lyx_output(GString *out, node* list, scratch_pad *scratch) {
	node *content;
	char *value;
#ifdef DEBUG_ON
	fprintf(stderr, "end_lyx_output\n");
#endif
	

     	/* Handle BibTeX */
	
if (tree_contains_key(list, METAKEY)) {
		content = metadata_for_key("bibtex",list);
		if (content != NULL) {
			g_string_append(out, "\n\\begin_layout Standard\n");
			g_string_append(out,"\n\\begin_inset CommandInset bibtex");
			g_string_append(out,"\nLatexCommand bibtex");
			value = metavalue_for_key("bibtex",list);
			g_string_append_printf(out,"\nbibfiles \"%s\"",value);
			free(value);
			g_string_append(out,"\noptions \"plainnat\"");
			g_string_append(out,"\n\n\\end_inset");
			g_string_append(out, "\n\n\\end_layout\n");
		}
	}	

	g_string_append(out, "\n\\end_body\n");
	g_string_append(out, "\\end_document\n");
}

bool is_lyx_complete_doc(node *meta);


/* print_lyx_node_tree -- convert node tree to Lyx */
void print_lyx_node_tree(GString *out, node *list, scratch_pad *scratch, bool no_newline) {
#ifdef DEBUG_ON
    int i;
    scratch->lyx_debug_nest++;
    g_string_append(scratch->lyx_debug_pad,"  "); /* add a level */
	fprintf(stderr, "\n%sStart_print_Node_Tree: %s\n",scratch->lyx_debug_pad->str,node_types[scratch->lyx_para_type]);
	scratch->lyx_debug_nest++;
    g_string_append(scratch->lyx_debug_pad,"  "); /* add a level */
#endif
	while (list != NULL) {
		print_lyx_node(out, list, scratch, no_newline);
		list = list->next;
	}
#ifdef DEBUG_ON
    scratch->lyx_debug_nest--;
	g_string_free(scratch->lyx_debug_pad,TRUE); /* don' see a way to shorten the string */
	scratch->lyx_debug_pad = g_string_new(""); /* so create a new, shorter one */
	for(i=0;i<scratch->lyx_debug_nest;i++)
	  g_string_append(scratch->lyx_debug_pad,"  ");
	fprintf(stderr, "\n%sEnd_print_Node_Tree: %s\n",scratch->lyx_debug_pad->str,node_types[scratch->lyx_para_type]);
	scratch->lyx_debug_nest--;
	g_string_free(scratch->lyx_debug_pad,TRUE); /* don' see a way to shorten the string */
	scratch->lyx_debug_pad = g_string_new(""); /* so create a new, shorter one */
	for(i=0;i<scratch->lyx_debug_nest;i++)
	  g_string_append(scratch->lyx_debug_pad,"  ");
#endif	
}

/* print_lyx_node -- convert given node to Lyx and append */
void print_lyx_node(GString *out, node *n, scratch_pad *scratch, bool no_newline) {
	node *temp_node;
	node *tcaption;
	char *temp;
	char *prefixed_label;
	int lev;
	char *width = NULL;
	char *height = NULL;
	GString *temp_str;
	GString *raw_str;
	char char_temp;
	int i;
	int old_type;
	int rows;
	int cols;
	int multicol;
	int colwidth;

	if (n == NULL)
		return;
	
	/* debugging statement */
#ifdef DEBUG_ON
	fprintf(stderr, "\n%sprint_lyx_node: %s\n",scratch->lyx_debug_pad->str,node_types[n->key]);
	fprintf(stderr,"%scontent: %s\n",scratch->lyx_debug_pad->str,n->str);
#endif
	
	switch (n->key) {
		case NO_TYPE:
			break;
		case LIST:
			print_lyx_node_tree(out,n->children,scratch, no_newline);
			break;
		case STR:
			print_lyx_string(out,n->str, scratch,LYX_NONE);
			break;
		case ABBREVIATION: 
		   // this work was done in writer.c
			break;
		case ABBRSTART:
			/* Strip out nodes that are being replaced with the abbreviation */
			temp_node = n->next;
			while (temp_node->key != ABBRSTOP) {
				n->next = temp_node->next;
				temp_node->next = NULL;
				free_node(temp_node);
				temp_node = n->next;
			}
			n->next = temp_node->next;
			temp_node->next = NULL;
			free_node(temp_node);
		case ABBR:
			/* In either case, now we call on the abbreviation */
//			width = ascii_label_from_node(n->children->children);
            width = string_from_node_tree(n->children->children);
//			temp = ascii_label_from_string(n->children->str);
			temp_str = g_string_new("");
		    g_string_append_printf(temp_str,"[%s]",width);
		    if(strstr(used_abbreviations->str,temp_str->str)){
		    	g_string_append(out,width); // just the abbrev
		    }
		    else
		    {
		    g_string_append(used_abbreviations,temp_str->str);
		      
			
			g_string_append(out,n->children->str);
			g_string_append_printf(out," (%s)",width);
			
			
			g_string_append(out,"\n\\begin_inset CommandInset nomenclature");
			g_string_append(out,"\nLatexCommand nomenclature");
     		g_string_append_printf(out,"\nsymbol \"%s\"",width);
			g_string_append(out,"\ndescription \"");
//            g_string_append(out,n->children->str);
            temp = escape_string(n->children->str);
            g_string_append(out,temp);
			g_string_append(out,"\"");		
			g_string_append(out, "\n\\end_inset\n");
			free(temp);
            }
            g_string_free(temp_str,TRUE);
			free(width);
			break;
		case ABBRSTOP:
			break;
		case SPACE:
			if (strncmp(n->str,"\n",1)==0){
			  if (no_newline){
			  	g_string_append(out," "); /* just a space */
			  } else{
			    g_string_append_printf(out,"%s ",n->str); /* lyx needs the space */
		      }
			}else{
			  g_string_append_printf(out,"%s",n->str);
			}
			break;
		case PLAIN: /* act as if all items are wrapped in a paragraph */
		case PARA:
			#ifdef DEBUG_ON
	        fprintf(stderr, "\n%sprint_lyx_paragraph: %s\n",scratch->lyx_debug_pad->str,node_types[scratch->lyx_para_type]);
	        fprintf(stderr,"%scontent: %s\n",scratch->lyx_debug_pad->str,n->str);
            #endif
			/* definition list special case, must append first definition to the term */
			if (scratch -> lyx_para_type == DEFINITION){
				  if (!scratch->lyx_definition_open){ /* first definition after a term */
				    g_string_append(out,"\n ");
				    print_lyx_node_tree(out,n->children,scratch, FALSE); /* stick on the end of the term */
			        g_string_append(out,"\n\\end_layout\n");
				    scratch->lyx_definition_open = TRUE; /* first definition after a term hit */
				    } else{
				    g_string_append(out,"\n\n\\begin_deeper\n");  // second (or nth definition)
				    g_string_append(out, "\n\\begin_layout Standard\n"); /* treat it as a paragraph */
				    print_lyx_node_tree(out,n->children,scratch, FALSE);
			        g_string_append(out, "\n\\end_layout\n");
			        g_string_append(out,"\n\\end_deeper\n");
				    }
				break;
			}
			switch (scratch->lyx_para_type) {
				case BLOCKQUOTE:
					g_string_append(out, "\n\\begin_layout Quote\n");
					break;
				case ORDEREDLIST:
					g_string_append(out, "\n\\begin_layout Enumerate\n");
			    	if (scratch-> lyx_beamerbullet){
					  g_string_append(out,"\n\\begin_inset Argument 1");
					  g_string_append(out,"\nstatus open\n");
					  g_string_append(out,"\n\\begin_layout Plain Layout");
					  g_string_append(out,"\n<+->");
					  g_string_append(out,"\n\\end_layout\n");
					  g_string_append(out,"\n\\end_inset\n");
					  scratch -> lyx_beamerbullet = FALSE;
					}
					break;
				case BULLETLIST:
					g_string_append(out, "\n\\begin_layout Itemize\n");
					if (scratch-> lyx_beamerbullet){
					  g_string_append(out,"\n\\begin_inset Argument 1");
					  g_string_append(out,"\nstatus open\n");
					  g_string_append(out,"\n\\begin_layout Plain Layout");
					  g_string_append(out,"\n+-");
					  g_string_append(out,"\n\\end_layout\n");
					  g_string_append(out,"\n\\end_inset\n");
					  scratch -> lyx_beamerbullet = FALSE;
					}
					break;
				case NOTEREFERENCE:
				case CITATION:
				case NOCITATION:
					g_string_append(out, "\n\\begin_inset Foot");
					g_string_append(out, "\nstatus collapsed\n");
					g_string_append(out, "\n\\begin_layout Plain Layout");
					break;
				case NOTESOURCE:
			    case CITATIONSOURCE:
			    	break; /* no enclosure by an environment */
			    case GLOSSARYSOURCE:
			    	g_string_append(out,"\ndescription \"");
			    	break;
				default:
					g_string_append(out, "\n\\begin_layout Standard\n");
					break;
			}
			print_lyx_node_tree(out,n->children,scratch, FALSE);
			if (scratch->lyx_para_type == GLOSSARYSOURCE){
				   g_string_append(out,"\"\n");
			} else if ((scratch->lyx_para_type != NOTESOURCE) &&
			        (scratch->lyx_para_type != CITATIONSOURCE)){
			       g_string_append(out, "\n\\end_layout\n");
	        }
			if ((scratch->lyx_para_type == CITATION) ||
			    (scratch->lyx_para_type == NOCITATION)) {
			  g_string_append(out,"\n\\end_layout\n");
			  g_string_append(out,"\n\\end_inset");
			}
			break;
		case HRULE:
			g_string_append(out,"\n\\begin_layout Standard\n");
			g_string_append(out,"\n\\begin_inset CommandInset line\n");
			g_string_append(out,"LatexCommand rule\n");
			g_string_append(out,"offset \"0.5ex\"\n");
			g_string_append(out,"width \"100col%\"\n");
			g_string_append(out,"height \"1pt\"\n");
			g_string_append(out,"\n\\end_inset\n");
			g_string_append(out,"\n\\end_layout\n");
			break;
		case HTMLBLOCK:
			/* don't print HTML block */
			/* but do print HTML comments for raw LaTeX */
			if (strncmp(n->str,"<!--",4) == 0) {
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append(out, "\n\\begin_layout Plain Layout\n\\begin_inset ERT\nstatus collapsed\n\n\\begin_layout Plain Layout\n\n");
                print_latex_string(out,&n->str[4],scratch);				
				g_string_append(out,"\n\n\\end_layout\n\n\\end_inset\n\\end_layout\n");
			}
			break;
		case VERBATIM:
		case VERBATIMFENCE:
			old_type = scratch->lyx_para_type;
			scratch->lyx_para_type = VERBATIM;
			scratch->lyx_level++;
			if (scratch->lyx_level > 1){
				g_string_append(out,"\n\\begin_deeper\n");
			}
			g_string_append(out,"\\begin_layout Standard\n");
			g_string_append(out,"\\begin_inset listings\n");
			if ((n->children != NULL) && (n->children->key == VERBATIMTYPE)) {
				trim_trailing_whitespace(n->children->str);
				if (strlen(n->children->str) > 0) {
					// NOTE: the language must match the LyX (LaTex) languages (e.g: Perl, not perl)
					g_string_append_printf(out, "lstparams \"basicstyle={\\footnotesize\\ttfamily},language=%s\"\n", n->children->str,n->str);
				}
			   else {
			   	 	g_string_append(out,"lstparams \"basicstyle={\\footnotesize\\ttfamily}\"\n");
			   }
			} else {
		 	    g_string_append(out,"lstparams \"basicstyle={\\footnotesize\\ttfamily}\"\n");
		       }
			g_string_append(out,"inline false\n");
			g_string_append(out,"status collapsed\n");
			print_lyx_string(out, n->str, scratch,LYX_PLAIN); /* it is not children - just \n separated lines */
			g_string_append(out,"\n\\end_inset\n");
			g_string_append(out,"\\end_layout\n");
			 scratch->lyx_level--;
		    if (scratch->lyx_level > 0){   
		        g_string_append(out,"\n\\end_deeper\n");
		    }
			scratch->lyx_para_type = old_type;
			break;
		case BULLETLIST:
		case ORDEREDLIST:
		case DEFLIST:
			if (n->key == DEFLIST){
				scratch->lyx_definition_hit = TRUE; 
	            scratch->lyx_definition_open = FALSE;
			}
			old_type = scratch->lyx_para_type;
			scratch->lyx_para_type = n->key;
			scratch->lyx_level++;
			if (scratch->lyx_level > 1){
				g_string_append(out,"\n\\begin_deeper\n");
			}
			print_lyx_node_tree(out, n->children, scratch, FALSE);
		    scratch->lyx_level--;
		    if (scratch->lyx_level > 0){   
		        g_string_append(out,"\n\\end_deeper\n");
		    }
			scratch->lyx_para_type = old_type;
		    scratch->lyx_definition_open = FALSE;
			break;
		case LISTITEM:
			#ifdef DEBUG_ON
	          fprintf(stderr, "\nStart List Item\n");
            #endif
            old_type = scratch->lyx_para_type;
			temp_node = n-> children; /* should be a list node */
			if(temp_node->children == NULL) {
				g_string_append(out,"\n\\begin_layout Itemize\n\\end_layout\n"); /* empty list item */
			}
			else { 
				i = 0;
				temp_node = temp_node-> children; /* process the list */
				/* the first paragraph is the list item's first paragraph */
				print_lyx_node(out,temp_node,scratch,no_newline);
				/* now process any other content, including additional lists */
					temp_node = temp_node-> next;
					while ((temp_node != NULL) && (temp_node->key != BULLETLIST)
					       && (temp_node->key != ORDEREDLIST) && (temp_node->key != DEFLIST)){
					  i++;
					  if (i == 1){
					    g_string_append(out,"\n\\begin_deeper\n");
					    old_type = scratch->lyx_para_type;
					    scratch->lyx_para_type = PARA; /* and make it a paragraph, not a list item */
					  }
					  print_lyx_node(out, temp_node, scratch, no_newline);
	       		      temp_node = temp_node->next;
	       		    }
			    if (i>0){
			      i--;
			      scratch->lyx_para_type = old_type; /* reset the paragraph type */
			      g_string_append(out,"\n\\end_deeper\n");
		   	    } 
		        if (temp_node != NULL){ /* have hid an imbedded list */
			      print_lyx_node(out,temp_node,scratch,no_newline);
			    }
    	    }  
			#ifdef DEBUG_ON
	          fprintf(stderr, "\nEnd List Item\n");
            #endif
			break;
		case METADATA:
			/* metadata we care about already handled */
			break;
		case METAKEY:
		   	/* metadata we care about already handled */
			break;
		case METAVALUE:
          /* metadata we care about already handled */
			break;
		case FOOTER:
			print_lyx_endnotes(out, scratch);
			break;
		case HEADINGSECTION:
			print_lyx_node_tree(out,n->children,scratch , FALSE);
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;  /* assumes H1 ... H6 are in order */
			if (lev > 7)
				lev = 7;	/* Max at level 7 */
			GString *environment = g_string_new("\n\\begin_layout ");			
			g_string_append(environment,heading_name[lev-1]->str); /* get the (possibly user modified) section name */
			
				if (!scratch->lyx_number_headers){
				g_string_append(environment,"*\n");} /* mark as unnumbered */
				else{	
				g_string_append(environment,"\n");
		     	};
			/* Begin the environment */
			    g_string_append_printf(out,"%s",environment->str);
			g_string_free(environment,true);
			/* Don't allow footnotes */
			scratch->no_lyx_footnote = TRUE;
			if (n->children->key == AUTOLABEL) {
				/* use label for header since one was specified (MMD)*/
				temp = label_from_string(n->children->str);
				prefixed_label = prefix_label(heading_name[lev-1]->str,temp,FALSE);
				print_lyx_node_tree(out, n->children->next, scratch , FALSE);
				g_string_append(out,"\n\\begin_inset CommandInset label\n");
				g_string_append(out,"LatexCommand label\n");
				g_string_append_printf(out, "name \"%s\"",prefixed_label);
				g_string_append(out,"\n\\end_inset\n");
				free(prefixed_label);
				free(temp);
			} else {
				/* generate a label by default for MMD */
				temp = label_from_node_tree(n->children);
				prefixed_label = prefix_label(heading_name[lev-1]->str,temp,FALSE);
				print_lyx_node_tree(out, n->children, scratch, FALSE);
				g_string_append(out,"\n\\begin_inset CommandInset label\n");
				g_string_append(out,"LatexCommand label\n");
				g_string_append_printf(out, "name \"%s\"",prefixed_label);
				g_string_append(out,"\n\\end_inset\n");
				free(prefixed_label);
				free(temp);
			}
			scratch->no_lyx_footnote = FALSE;
			g_string_append(out,"\n\\end_layout\n");
			break;
		case APOSTROPHE:
			print_lyx_localized_typography(out, APOS, scratch);
			break;
		case ELLIPSIS:
			print_lyx_localized_typography(out, ELLIP, scratch);
			break;
		case EMDASH:
			print_lyx_localized_typography(out, MDASH, scratch);
			break;
		case ENDASH:
			print_lyx_localized_typography(out, NDASH, scratch);
			break;
		case SINGLEQUOTED:
			print_lyx_localized_typography(out, LSQUOTE, scratch);
			print_lyx_node_tree(out, n->children, scratch, FALSE);
			print_lyx_localized_typography(out, RSQUOTE, scratch);
			break;
		case DOUBLEQUOTED:
			print_lyx_localized_typography(out, LDQUOTE, scratch);
			print_lyx_node_tree(out, n->children, scratch, FALSE);
			print_lyx_localized_typography(out, RDQUOTE, scratch);
			break;
		case LINEBREAK:
			g_string_append(out, "\n\\begin_inset Newline newline\n\\end_inset\n");
			break;
		case MATHSPAN:
			if (n->str[0] == '$') {
				if (n->str[1] == '$') {
					if (strncmp(&n->str[2],"\\begin",5) == 0) {
						n->str[strlen(n->str)-2] = '\0';
						g_string_append_printf(out, "\n\\begin_inset Formula %s\n\\end_inset\n",&n->str[1]);
					} else {
						g_string_append_printf(out, "\n\\begin_inset Formula %s\n\\end_inset\n",n->str);
					}
				} else {
					if (strncmp(&n->str[1],"\\begin",5) == 0) {
						n->str[strlen(n->str)-1] = '\0';
						g_string_append_printf(out, "\n\\begin_inset Formula %s\n\\end_inset\n",&n->str[1]);
					} else {
						g_string_append_printf(out, "\n\\begin_inset Formula %s\n\\end_inset\n",n->str);
					}
				}
			} else if (strncmp(&n->str[2],"\\begin",5) == 0) {
				/* trim */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "\n\\begin_inset Formula %s\n\\end_inset\n", &n->str[2]);
			} else {
				if (n->str[strlen(n->str)-1] == ']') {
					n->str[strlen(n->str)-3] = '\0';
					g_string_append(out,"\\begin_inset Formula \n\\[");
					g_string_append_printf(out, "\n%s\n\\]\n\\end_inset\n", &n->str[2]);
				} else {
					n->str[strlen(n->str)-3] = '\0';
					g_string_append_printf(out, "\n\\begin_inset Formula $%s$\n\\end_inset\n", &n->str[2]);
				}
			}
			break;
		case STRONG:
			g_string_append(out, "\n\\series bold\n");
			print_lyx_node_tree(out,n->children,scratch, FALSE);
			g_string_append(out, "\n\\series default\n");
			break;
		case EMPH:
			g_string_append(out, "\n\\emph on\n");
			print_lyx_node_tree(out,n->children,scratch, FALSE);
			g_string_append(out, "\n\\emph default\n");
			break;
		case LINKREFERENCE:
			break;
		case LINK:
#ifdef DEBUG_ON
	fprintf(stderr, "print LyX link: '%s'\n",n->str);
#endif
           
			/* Do we have proper info? */

			if (n->link_data == NULL) {
				/* NULL link_data could occur if we parsed this link before and it didn't
					match anything */
				n->link_data =  mk_link_data(NULL, NULL, NULL, NULL);
			}

			if ((n->link_data->label == NULL) &&
			(n->link_data->source == NULL)) {
#ifdef DEBUG_ON
	fprintf(stderr, "print LyX link: '%s'\n",n->str);
#endif
				/* we seem to be a [foo][] style link */
				/* so load a label */
				temp_str = g_string_new("");
				print_raw_node_tree(temp_str, n->children);
				/* replace new-lines with spaces */
				temp = temp_str->str;
				while (*temp != '\0'){
					if (*temp == '\n'){
						*temp = ' ';
					}
					temp++;
				}
				free(n->link_data->label);
				n->link_data->label = temp_str->str;
				g_string_free(temp_str, FALSE);
			}
#ifdef DEBUG_ON
	fprintf(stderr, "look for reference data for LyX link: '%s'\n",n->str);
#endif
			/* Load reference data */
			if (n->link_data->label != NULL) {
#ifdef DEBUG_ON
	fprintf(stderr, "have label for LyX link: '%s'\n",n->str);
#endif
				temp = strdup(n->link_data->label);
				free_link_data(n->link_data);
				n->link_data = extract_link_data(temp, scratch);
				   
				if (n->link_data == NULL) {
					/* replace original text since no definition found */
					g_string_append(out, "[");
					print_lyx_node(out, n->children, scratch, FALSE);
					g_string_append(out,"]");
					if (n->children->next != NULL) {
						g_string_append(out, "[");
						print_lyx_node_tree(out, n->children->next, scratch, FALSE);
						g_string_append(out,"]");
					} else if (n->str != NULL) {
						/* no title label, so see if we stashed str*/
						g_string_append_printf(out, "%s", n->str);
					} else {
						g_string_append_printf(out, "[%s]",temp);
					}
					free(temp);
					break;
				}
				free(temp);
			}
			temp_str = g_string_new("");
			print_lyx_node_tree(temp_str, n->children, scratch, TRUE);
			raw_str = g_string_new("");
			print_raw_node_tree(raw_str, n->children);
			/* replace new-lines with spaces */
				temp = raw_str->str;
				while (*temp != '\0'){
					if (*temp == '\n'){
						*temp = ' ';
					}
					temp++;
				}	
			if ((n->link_data->source != NULL) && (n->link_data->source[0] == '#' )) {
				   
				/* link to anchor within the document */
				if (strlen(temp_str->str) > 0) {
					/* We have text before the link */
					g_string_append_printf(out, "%s (", temp_str->str);
				}
				
				if (n->link_data->label == NULL) {
					if ((n->link_data->source !=  NULL) && (n->link_data->source[0] == '#' )) {
						/* This link was specified as [](#bar) */
						g_string_append(out,"\n\\begin_inset CommandInset ref");
						g_string_append_printf(out,"\nLatexCommand formatted");
						g_string_append_printf(out,"\nreference \"%s\"\n",n->link_data->source + 1);
						g_string_append(out,"\n\\end_inset\n");

					} else {
                        g_string_append_printf(out, "\n\\begin_inset CommandInset href\nLatexCommand href\ntarget \"%s\"\n", n->link_data->source);
						g_string_append(out, "\"\n\n\\end_inset\n");
	
					}
				} else {
					g_string_append(out,"\n\\begin_inset CommandInset ref");
					g_string_append_printf(out,"\nLatexCommand formatted");
					g_string_append_printf(out,"\nreference \"%s\"\n",n->link_data->source + 1);
					g_string_append(out,"\n\\end_inset\n");
				}
				if (strlen(temp_str->str) > 0) {
					g_string_append(out, ")");
				}
			} else if (strcmp(raw_str->str, n->link_data->source) == 0){
				/* This is a <link> */
	            g_string_append_printf(out, "\n\\begin_inset CommandInset href\nLatexCommand href\ntarget \"%s\"\n", n->link_data->source);
				g_string_append_printf(out,"name \"%s\"",temp_str->str);
				g_string_append(out, "\n\n\\end_inset\n");
			} else if (strcmp(raw_str->str,&n->link_data->source[7]) == 0) {
				/*This is a <mailto> */
                g_string_append_printf(out, "\n\\begin_inset CommandInset href\nLatexCommand href\ntarget \"%s\"\n", n->link_data->source);
				g_string_append_printf(out,"name \"%s\"",temp_str->str);
				g_string_append(out,"\ntype \"mailto:\"");
				g_string_append(out, "\n\n\\end_inset\n");
			} else {
				g_string_append_printf(out, "\n\\begin_inset CommandInset href\nLatexCommand href\ntarget \"%s\"\n", n->link_data->source);
				g_string_append(out,"name \"");
				
				g_string_free(temp_str,TRUE);
				temp_str = g_string_new("");
				print_escaped_node_tree(out,n->children);
                
				g_string_append(out, "\"\n\n\\end_inset\n");
				if (scratch->no_lyx_footnote == FALSE) {
					g_string_append(out, "\n\\begin_inset Foot\nstatus collapsed\n\n\\begin_layout Plain Layout\n");
					g_string_append(out, "\n\\begin_inset CommandInset href\nLatexCommand href\n");
					g_string_append_printf(out,"\nname \"%s\"",n->link_data->source);
					g_string_append_printf(out,"\ntarget \"%s\"",n->link_data->source);
                    g_string_append(out,"\n\n\\end_inset");
					g_string_append(out, "\n\\end_layout\n\n\\end_inset\n");
				}
			}
			g_string_free(temp_str, TRUE);
			g_string_free(raw_str, true);
			n->link_data->attr = NULL;
			break;
		case ATTRKEY:
			g_string_append_printf(out, " %s=\"%s\"", n->str,
				n->children->str);
			break;
		case REFNAME:
		case SOURCE:
		case TITLE:
			break;
		case IMAGEBLOCK:
		case IMAGE:
#ifdef DEBUG_ON
	fprintf(stderr, "print image\n");
#endif
			/* Do we have proper info? */
			if ((n->link_data->label == NULL) &&
			(n->link_data->source == NULL)) {
				/* we seem to be a [foo][] style link */
				/* so load a label */
				temp_str = g_string_new("");
				print_raw_node_tree(temp_str, n->children);
				n->link_data->label = temp_str->str;
				g_string_free(temp_str, FALSE);
			}
			/* Load reference data */
			if (n->link_data->label != NULL) {
				temp = strdup(n->link_data->label);
				free_link_data(n->link_data);
				n->link_data = extract_link_data(temp, scratch);
				    
				if (n->link_data == NULL) {
					/* replace original text since no definition found */
					g_string_append(out, "![");
					print_lyx_node(out, n->children, scratch, FALSE);
					g_string_append(out,"]");
					if (n->children->next != NULL) {
						g_string_append(out, "[");
						print_lyx_node_tree(out, n->children->next, scratch, FALSE);
						g_string_append(out,"]");
					} else if (n->str != NULL) {
						/* no title label, so see if we stashed str*/
						g_string_append_printf(out, "%s", n->str);
					} else {
						g_string_append_printf(out, "[%s]",temp);
					}
					free(temp);
					break;
				}
				free(temp);
			}
			
			if (n->key == IMAGEBLOCK){
				g_string_append(out,"\n\\begin_layout Standard"); /* needs to be in an environment */
				g_string_append(out,"\n\\begin_inset Float figure");
				g_string_append(out,"\nwide false");
				g_string_append(out,"\nsideways false");
				g_string_append(out,"\nstatus collapsed");
				g_string_append(out, "\n\n\\begin_layout Plain Layout");
			}
			
			g_string_append(out,"\n\\begin_inset Graphics");
			
			g_string_append_printf(out, "\n\t filename %s\n",n->link_data->source);

#ifdef DEBUG_ON
	fprintf(stderr, "attributes\n");
#endif

			if (n->link_data->attr != NULL) {
				temp_node = node_for_attribute("height",n->link_data->attr);
				if (temp_node != NULL)
					height = correct_dimension_units(temp_node->children->str);
				temp_node = node_for_attribute("width",n->link_data->attr);
				if (temp_node != NULL)
					width = correct_dimension_units(temp_node->children->str);
			}
				
				if (width != NULL) {
					if (width[strlen(width)-1] == '%') {
						width[strlen(width)-1] = '\0';
						g_string_append_printf(out, "\tlyxscale %s\n", width);
						g_string_append_printf(out, "\tscale %s\n",width);
					} else {
						g_string_append_printf(out, "\twidth %s\n",width);
					}
				}
				
				if (height != NULL) {
					if (height[strlen(height)-1] == '%') {
						height[strlen(height)-1] = '\0';
						g_string_append_printf(out, "\tlyxscale %s\n",height);
						g_string_append_printf(out, "\tscale %s\n",height);
					} else {
						g_string_append_printf(out, "\theight %s\n",height);
					}
				}
			g_string_append(out,"\n\\end_inset\n");
			
			if (n->key == IMAGEBLOCK) {
				g_string_append(out,"\n\n\\end_layout\n");
				if (n->children != NULL) {
				g_string_append(out,"\n\\begin_layout Plain Layout");
				g_string_append(out,"\n\\begin_inset Caption");
				g_string_append(out,"\n\n\\begin_layout Plain Layout\n");
					print_lyx_node_tree(out, n->children, scratch, FALSE);
				g_string_append(out,"\n\\end_layout\n");
			    g_string_append(out,"\n\\end_inset");
				if (n->link_data->label != NULL) {
					g_string_append(out,"\n\n\\begin_inset CommandInset label");
					g_string_append(out,"\nLatexCommand label\n");
					temp = label_from_string(n->link_data->label);
				    g_string_append_printf(out, "\nname \"fig:%s\"",temp);
					g_string_append(out,"\n\\end_inset");
					free(temp);
				}
			    g_string_append(out,"\n\\end_layout\n");
			    g_string_append(out,"\n\\end_inset\n");
			}
					
				g_string_append(out, "\n\\end_layout\n");
			}
			
			free(height);
			free(width);
			n->link_data->attr = NULL;	/* We'll delete these elsewhere */
			break;
#ifdef DEBUG_ON
	fprintf(stderr, "finish image\n");
#endif
		case NOTEREFERENCE:
			lev = note_number_for_node(n, scratch);
			temp_node = node_for_count(scratch->used_notes, lev);
			if (temp_node->key == GLOSSARYSOURCE) {
				g_string_append(out,"\n\\begin_inset CommandInset nomenclature");
			    g_string_append(out,"\nLatexCommand nomenclature");
			    scratch->lyx_para_type = temp_node->key;
				print_lyx_node_tree(out, temp_node->children, scratch, FALSE);
				scratch->lyx_para_type = NO_TYPE; 
				g_string_append(out, "\n\\end_inset\n");
			} else {
				g_string_append(out, "\n\\begin_inset Foot");
				g_string_append(out,"\nstatus collapsed\n\n");
				old_type = scratch->lyx_para_type;
				scratch->lyx_para_type = PARA;
				print_lyx_node_tree(out, temp_node->children, scratch, FALSE);
				scratch->lyx_para_type = old_type;
				g_string_append(out, "\n\n\\end_inset\n");
			}
			break;
		case NOCITATION:
		case CITATION:
#ifdef DEBUG_ON
	fprintf(stderr, "\nprint cite\n");
#endif
			if ((n->link_data != NULL) && (strncmp(n->link_data->label,"[#",2) == 0)) {
				/* external citation (e.g. BibTeX) */
				n->link_data->label[strlen(n->link_data->label)-1] = '\0';
				if (n->key == NOCITATION) {
					g_string_append(out,"\n\\begin_inset CommandInset citation");
					g_string_append(out,"\nLatexCommand nocite");
					g_string_append_printf(out,"\nkey \"%s\"",&n->str[2]);
					g_string_append(out,"\n\n\\end_inset\n");
				} else {
					g_string_append(out, "<FAKE span class=\"externalcitation\">");
					g_string_append(out, "</span>");
				}
			} else {
#ifdef DEBUG_ON
				fprintf(stderr, "internal cite\n");
#endif
				/* MMD citation, so output as footnote */
				/* TODO: create separate stream from footnotes */
				lev = note_number_for_label(n->link_data->label, scratch);
				if (lev != 0) {
#ifdef DEBUG_ON
					fprintf(stderr, "matching cite found\n");
#endif
					temp_node = node_for_count(scratch->used_notes, lev);
					/* flag that this is used as a citation */
					temp_node->key = CITATIONSOURCE;
					if (lev > scratch->max_footnote_num) {
						scratch->max_footnote_num = lev;
					}
					if (n->key == NOCITATION) {
						g_string_append(out,"\n\\begin_inset CommandInset citation");
						g_string_append(out,"\nLatexCommand nocite");
						g_string_append_printf(out,"\nkey \"%s\"",n->link_data->label);
						g_string_append(out,"\n\n\\end_inset\n");
					} else {
						if (n->children != NULL) {
							g_string_append(out,"\n\\begin_inset CommandInset citation");
							g_string_append(out,"\nLatexCommand cite");
							g_string_append(out, "\nafter \"");
							print_lyx_node(out, n->children, scratch, FALSE);
							g_string_append_printf(out,"\"\nkey \"%s\"",n->link_data->label);
							g_string_append(out,"\n\n\\end_inset\n");
						} else {
							g_string_append(out,"\n\\begin_inset CommandInset citation");
							g_string_append(out,"\nLatexCommand cite");
							g_string_append_printf(out,"\nkey \"%s\"",n->link_data->label);
							g_string_append(out,"\n\n\\end_inset\n");
						}
					}
				} else {
					/* not located -- this is external cite */
#ifdef DEBUG_ON
				fprintf(stderr, "no match for cite: '%s'\n",n->link_data->label);
#endif
					temp = n->link_data->label;
					if (n->key == NOCITATION) {
						g_string_append(out,"\n\\begin_inset CommandInset citation");
						g_string_append(out,"\nLatexCommand nocite");
						g_string_append_printf(out,"\nkey \"%s\"",n->link_data->label);
						g_string_append(out,"\n\n\\end_inset\n");
					} else {
						if (n->children != NULL) {
#ifdef DEBUG_ON
				fprintf(stderr, "cite with children\n");
#endif
							if (strcmp(&temp[strlen(temp) - 1],";") == 0) {
								g_string_append(out, " \\citet[");
								temp[strlen(temp) - 1] = '\0';
							} else {
								g_string_append(out,"\n\\begin_inset CommandInset citation");
							    g_string_append(out,"\nLatexCommand cite");
							    g_string_append(out, "\nafter \"");	
							}
							print_lyx_node(out, n->children, scratch, FALSE);
							g_string_append_printf(out,"\"\nkey \"%s\"",temp);
							g_string_append(out,"\n\n\\end_inset\n");
						} else {
#ifdef DEBUG_ON
				fprintf(stderr, "cite without children. locat:'%s'\n",n->str);
#endif
							if (strcmp(&temp[strlen(temp) - 1],";") == 0) {
								temp[strlen(temp) - 1] = '\0';
								g_string_append_printf(out, " \\citet{%s}",temp);
							} else {
								g_string_append(out,"\n\\begin_inset CommandInset citation");
							    g_string_append(out,"\nLatexCommand cite");
							    g_string_append_printf(out,"\nkey \"%s\"",temp);
							    g_string_append(out,"\n\n\\end_inset\n");
							}
						}
					}
				}
			}
#ifdef DEBUG_ON
		fprintf(stderr, "finish cite\n");
#endif
			break;
		case VARIABLE:
			temp = metavalue_for_key(n->str,scratch->result_tree);
			if (temp == NULL) {
				g_string_append_printf(out, "[%%%s]",n->str);
			} else {
				g_string_append_printf(out, temp);
				free(temp);
			}
			break;
		case GLOSSARYTERM:
			if ((n->next != NULL) && (n->next->key == GLOSSARYSORTKEY) ) {
				g_string_append(out, "\nprefix \"");
				print_lyx_string(out, n->next->str, scratch,LYX_NONE);
				g_string_append(out, "\"");
			}
			g_string_append(out,"\nsymbol \"");
			print_latex_string(out, n->children->str, scratch);
			g_string_append(out, "\"");
			break;
		case GLOSSARYSORTKEY:
			break;
		case CODE:
			g_string_append(out, "\n\\family typewriter\n");
			print_lyx_string(out, n->str, scratch,LYX_CODE);
			g_string_append(out, "\n\\family default\n");
			break;
		case BLOCKQUOTEMARKER:  					
		    print_lyx_node_tree(out, n->children, scratch, FALSE);
			break;
		case BLOCKQUOTE: /* can be nested - make it work like lists */
		    old_type = scratch->lyx_para_type;
			scratch->lyx_para_type = n->key;
			scratch->lyx_level++;
			if (scratch->lyx_level > 1){
				g_string_append(out,"\n\\begin_deeper\n");
			}
			print_lyx_node_tree(out, n->children, scratch, FALSE);
		    scratch->lyx_level--;
		    if (scratch->lyx_level > 0){   
		        g_string_append(out,"\n\\end_deeper\n");
		    }
			scratch->lyx_para_type = old_type;
			break;
		case RAW:
			/* This shouldn't happen */
			g_string_append(out, "RAW:");
			g_string_append_printf(out,"%s",n->str);
			break;
		case HTML:
			/* Handle HTML Reserved Characters */
			if (strncmp(n->str,"&quot;",6) == 0){
				g_string_append(out,"\"");
				break;
			} else if (strncmp(n->str,"&apos;",6) == 0){
				g_string_append(out,"'");
				break;
			} else if (strncmp(n->str,"&amp;",5) == 0){
				g_string_append(out,"&");
				break;
			} else if (strncmp(n->str,"&lt;",4) == 0){
				g_string_append(out,"<");
				break;
			} else if (strncmp(n->str,"&gt;",4) == 0){
				g_string_append(out,">");
				break;
			};
			
			/* don't print HTML block */
			/* but do print HTML comments for raw LaTeX */
			if (strncmp(n->str,"<!--",4) == 0) {
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
//				g_string_append(out, "\n\\begin_layout Plain Layout\n\\begin_inset ERT\nstatus collapsed\n\n\\begin_layout Plain Layout\n\n");
				g_string_append(out, "\n\\begin_inset ERT\nstatus collapsed\n\n\\begin_layout Plain Layout\n\n");
                print_lyx_string(out,&n->str[4],scratch,LYX_NONE);				
//              g_string_append(out,"\n\n\\end_layout\n\\end_inset\n\\end_layout\n");
				g_string_append(out,"\n\n\\end_layout\n\\end_inset\n");
			}
			break;
		case TERM:
			scratch->lyx_definition_open = FALSE; /* and it is closed */
			old_type = scratch->lyx_para_type;
			scratch->lyx_para_type = n->key;
			if (scratch->lyx_definition_hit){	
			  g_string_append(out,"\n\\begin_layout Labeling");
			  g_string_append(out,"\n\\labelwidthstring 00.00.0000\n");
			  g_string_append(out,"\n\\series bold\n");
			  scratch -> lyx_definition_hit = FALSE; /* waiting for it to start a new set of terms */
		    } else { /* multiple terms, join with commas */
		      	g_string_append(out,",");
		      	g_string_append(out,"\n\\begin_inset space ~\n\\end_inset\n");
		    }
			temp_str = g_string_new("");
			print_lyx_node_tree(temp_str, n->children, scratch, FALSE);
		    /* replace spaces with protected spaces */
				temp = temp_str->str;
				while (*temp != '\0'){
					if (*temp == ' '){
						g_string_append(out,"\n\\begin_inset space ~\n\\end_inset\n");
					} else{
						g_string_append_printf(out,"%c",*temp);
					}
					temp++;
				}
			g_string_free(temp_str,TRUE);
			scratch->lyx_para_type = old_type;
			break;
		case DEFINITION:
			if (!scratch -> lyx_definition_hit){
				g_string_append(out,"\n\\series default\n"); /* close bolding */
			}
			scratch -> lyx_definition_hit = TRUE;  /* have hit the definiton thus we can start a new one */
			old_type = scratch->lyx_para_type;
			scratch->lyx_para_type = n->key;
			print_lyx_node_tree(out, n->children, scratch, FALSE);
			scratch->lyx_para_type = old_type;
			break;
		case TABLE:
			scratch->lyx_table_caption = NULL;
			/* need to check the caption here in order to decide whether to increment the number of rows */
			tcaption = n->children;
			if (tcaption->key == TABLECAPTION) {
	           scratch->lyx_table_caption = tcaption;
			}
			scratch->table_row = 0;
			scratch->table_column = 0;
			lyx_get_table_dimensions(n->children,&rows,&cols,scratch);
			scratch->lyx_table_total_rows = rows;
			if (scratch->lyx_table_caption != NULL){
				rows++; /* caption goes on the first row */
			}
			scratch->lyx_table_total_cols = cols;
			g_string_append(out,"\n\\begin_layout Standard");
			g_string_append(out,"\n\\begin_inset Tabular");
			g_string_append_printf(out,"\n<lyxtabular version=\"3\" rows=\"%d\" columns=\"%d\">",rows, cols);
			g_string_append(out,"\n<features booktabs=\"true\" tabularvalignment=\"middle\" islongtable=\"true\" longtabularalignment=\"center\">");

			print_lyx_node_tree(out, n->children, scratch, FALSE); /* table body */
			g_string_append(out, "\n</lyxtabular>");
			g_string_append(out,"\n\\end_inset");
			g_string_append(out, "\n\\end_layout\n");
			scratch->lyx_table_caption = NULL;
			scratch->table_alignment = NULL;
			break;
		case TABLESEPARATOR:
			colwidth = 100/scratch->lyx_table_total_cols;
			scratch->table_alignment = n->str;
			for (i=0;i<scratch->lyx_table_total_cols;i++){
			  temp_str = g_string_new("");
			  char_temp = scratch->table_alignment[i];
			  switch(char_temp){
				case 'c':
				case 'C':
					g_string_append(temp_str,"center");
					break;
				case 'r':
				case 'R':
					g_string_append(temp_str,"right");
					break;
				case 'l':
				case 'L':
					g_string_append(temp_str,"left");
				    break;
		      }
			  g_string_append_printf(out,"\n<column alignment=\"%s\" valignment=\"top\" width=\"%dcol%%\">",temp_str->str,colwidth);
			  g_string_free(temp_str,TRUE);
			}
			break;
		case TABLECAPTION:
	        /* handled above */
			break;
		case TABLELABEL:
			break;
		case TABLEHEAD:
			if (scratch-> lyx_table_caption != NULL){ /* if there is a caption */
			  g_string_append(out,"\n<row caption=\"true\">");
			  g_string_append(out,"\n<cell multicolumn=\"1\" alignment=\"left\" valignment=\"top\" usebox=\"none\">");
			  g_string_append(out,"\n\\begin_inset Text\n");
			  g_string_append(out,"\n\\begin_layout Plain Layout");
			  g_string_append(out,"\n\\begin_inset Caption\n");
			  g_string_append(out,"\n\\begin_layout Plain Layout\n");
			  print_lyx_node_tree(out, scratch->lyx_table_caption->children, scratch, FALSE);
			  if ((scratch->lyx_table_caption->children != NULL) && (scratch->lyx_table_caption->children->key == TABLELABEL)) {
				temp = label_from_string(scratch->lyx_table_caption->children->str);
			    } else {
				temp = label_from_node_tree(scratch->lyx_table_caption->children);
			    }
			    g_string_append(out,"\n\\end_layout\n");
			    g_string_append(out,"\n\\end_inset");
			    g_string_append(out,"\n\n\\begin_inset CommandInset label");
				g_string_append(out,"\nLatexCommand label\n");
				g_string_append_printf(out, "\nname \"tab:%s\"",temp);
				g_string_append(out,"\n\\end_inset");
			  g_string_append(out,"\n\\end_layout\n");
			  g_string_append(out,"\n\\end_inset\n");
			  g_string_append(out,"\n</cell>");
			  for (i=0;i<scratch->lyx_table_total_cols-1;i++){
			  	g_string_append(out,"\n<cell multicolumn=\"2\" alignment=\"center\" valignment=\"top\" topline=\"true\" bottomline=\"true\" leftline=\"true\" usebox=\"none\">");
			  	g_string_append(out,"\n\\begin_inset Text\n");
			  	g_string_append(out,"\n\\begin_layout Plain Layout\n");
			  	g_string_append(out,"\n\\end_layout\n");
			  	g_string_append(out,"\n\\end_inset");
			  	g_string_append(out,"\n</cell>");
			  }
			  g_string_append(out,"\n</row>");
			  free(temp); 
			}
			scratch->lyx_table_need_line = TRUE;
			scratch->lyx_in_header = TRUE;
			print_lyx_node_tree(out, n->children, scratch, FALSE);
			scratch->lyx_in_header = FALSE;		
			break;
		case TABLEBODY:
			scratch->lyx_table_need_line = TRUE;
			print_lyx_node_tree(out, n->children, scratch, FALSE);
			break;
		case TABLEROW:
			if (scratch->lyx_in_header){
			    g_string_append(out, "\n<row endhead=\"true\" endfirsthead=\"true\">");	
			} else {
		     	g_string_append(out, "\n<row>");
		    }
			scratch->table_column = 0;
			print_lyx_node_tree(out, n->children, scratch, FALSE);
			g_string_append(out,"\n</row>");
			scratch->lyx_table_need_line = FALSE;
			scratch->table_row++;
			break;
		case TABLECELL:
			temp_str = g_string_new("");
			char_temp = scratch->table_alignment[scratch->table_column];
			switch(char_temp){
				case 'c':
				case 'C':
					g_string_append(temp_str,"center");
					break;
				case 'r':
				case 'R':
					g_string_append(temp_str,"right");
					break;
				case 'l':
				case 'L':
					g_string_append(temp_str,"left");
				    break;
			}
			multicol = 1;
			if ((n->children != NULL) && (n->children->key == CELLSPAN)){
				multicol = (int)strlen(n->children->str)+1;
			}
			for(i=1;i<=multicol;i++){

	            g_string_append(out,"\n<cell" ) ;
	            if (multicol > 1) {
					g_string_append_printf(out, " multicolumn=\"%d\"",i);
				}
				g_string_append_printf(out, " alignment=\"%s\"",temp_str->str);
				g_string_append(out, " valignment=\"top\"");
				
				if (scratch->lyx_table_need_line){
				   g_string_append(out," topline=\"true\"" );
			    }
			    if (scratch->table_row >= scratch->lyx_table_total_rows-1){
			       g_string_append(out," bottomline=\"true\"" );
			    }
			    
			    g_string_append(out," usebox=\"none\"");
			    
			    g_string_append(out,">");
			    
			    g_string_append(out,"\n\\begin_inset Text");
			    g_string_append(out,"\n\n\\begin_layout Plain Layout\n");
			    
				print_lyx_node_tree(out, n->children, scratch, FALSE);
				
				g_string_append(out,"\n\\end_layout\n");
			    g_string_append(out,"\n\n\\end_inset");
			    g_string_append(out,"\n</cell>");
		   }
			g_string_free(temp_str,TRUE);
			scratch->table_column++;
			break;
		case CELLSPAN:
			break;
		case GLOSSARYSOURCE: /* handled inline */
			break;
		case NOTESOURCE: /* handled inline */
		    break;
		case CITATIONSOURCE:
			scratch->lyx_para_type = n-> key;
			print_lyx_node(out, n->children, scratch, FALSE);
			break;
		case SOURCEBRANCH:
			fprintf(stderr,"SOURCEBRANCH\n");
			break;
		case NOTELABEL:
		case GLOSSARYLABEL:
			break;
		case SUPERSCRIPT:
			g_string_append_printf(out, "\n\\begin_inset script superscript\n\n\\begin_layout Plain Layout\n%s\n\\end_layout\n\n\\end_inset\n",n->str);
			break;
		case SUBSCRIPT:
			g_string_append_printf(out, "\n\\begin_inset script subscript\n\n\\begin_layout Plain Layout\n%s\n\\end_layout\n\n\\end_inset\n",n->str);
			break;
		case KEY_COUNTER:
			break;
		case TOC:
			print_lyx_node_tree(out,n->children, scratch, false);
			break;
		default:
			fprintf(stderr, "print_lyx_node encountered unknown node key = %d\n",n->key);
			exit(EXIT_FAILURE);
	}
}

/* print_lyx_endnotes */
void print_lyx_endnotes(GString *out, scratch_pad *scratch) {
	node *temp_node;
	bool do_nomenclature;
	scratch->used_notes = reverse_list(scratch->used_notes);
	node *note = scratch->used_notes;
#ifdef DEBUG_ON
	fprintf(stderr, "start endnotes\n");
#endif

    	    /* Handle Glossary or abbreviations */
    do_nomenclature = false;
	if (strcmp(used_abbreviations->str,"")!=0){  // if any abbreviations have been used, print a glossary
	  do_nomenclature = true; 
	} else
	{
      temp_node = note;
      while (temp_node != NULL){
      	  if(temp_node->key == GLOSSARYSOURCE){
          do_nomenclature = true;
    	  break;
          }
	  temp_node = temp_node->next;
	  }
    }
	
	if (do_nomenclature){
    	g_string_append(out,"\n\\begin_layout Standard");
    	g_string_append(out,"\n\\begin_inset CommandInset nomencl_print");
    	g_string_append(out,"\nLatexCommand printnomenclature");
    	g_string_append(out,"\nset_width \"auto\"\n");
    	g_string_append(out,"\n\\end_inset\n");
    	g_string_append(out,"\n\\end_layout\n");
	}

	if (note == NULL)
		return;
	
	note = scratch->used_notes;
	
#ifdef DEBUG_ON
	fprintf(stderr, "there are endnotes to print\n");
#endif

	while ( note != NULL) {
		if (note->key == KEY_COUNTER) {
			note = note->next;
			continue;
		}
		
		if (note->key == CITATIONSOURCE) {
			g_string_append(out, "\n\\begin_layout Bibliography\n");
			g_string_append(out,"\\begin_inset CommandInset bibitem\n");
			g_string_append(out,"LatexCommand bibitem\n");
			g_string_append_printf(out, "key \"%s\"\n", note->str);
			g_string_append_printf(out, "label \"%s\"\n", note->str);
			g_string_append(out,"\n\\end_inset\n");
			print_lyx_node(out, note, scratch, FALSE);
			g_string_append(out,"\n\\end_layout\n");
		} else {
			/* footnotes handled elsewhere */
		}

		note = note->next;
	}
	
	
#ifdef DEBUG_ON
	fprintf(stderr, "\n\\end_layout\n");
#endif
}

/* print_lyx_localized_typography -- convert to "smart" typography */
void print_lyx_localized_typography(GString *out, unsigned char character, scratch_pad *scratch) {
	if (!extension(EXT_SMART, scratch->extensions)) {
		g_string_append_c(out, character);
		return;
	}
	switch (character) {
		case LSQUOTE:
		case 0x91:
			switch (scratch->language) {
				case SWEDISH:
					g_string_append(out, "'");
					break;
				case FRENCH:
					g_string_append(out,"'");
					break;
				case GERMAN:
					g_string_append(out,"‚");
					break;
				case GERMANGUILL:
					g_string_append(out,"›");
					break;
				default:
					g_string_append(out,"\n\\begin_inset Quotes els\n\\end_inset\n");
				}
			break;
		case RSQUOTE:
		case 0x92:
			switch (scratch->language) {
				case GERMAN:
					g_string_append(out,"`");
					break;
				case GERMANGUILL:
					g_string_append(out,"‹");
					break;
				default:
				g_string_append(out,"\n\\begin_inset Quotes ers\n\\end_inset\n");
				}
			break;
		case APOS:
			g_string_append(out,"'");
			break;
		case LDQUOTE:
		case 0x93:
			switch (scratch->language) {
				case DUTCH:
				case GERMAN:
					g_string_append(out,"„");
					break;
				case GERMANGUILL:
					g_string_append(out,"»");
					break;
				case FRENCH:
					g_string_append(out,"«");
					break;
				case SWEDISH:
					g_string_append(out, "''");
					break;
				default:
					g_string_append(out,"\n\\begin_inset Quotes eld\n\\end_inset\n");
				}
			break;
		case RDQUOTE:
		case 0x94:
			switch (scratch->language) {
				case SWEDISH:
				case DUTCH:
					g_string_append(out,"''");
					break;
				case GERMAN:
					g_string_append(out,"``");
					break;
				case GERMANGUILL:
					g_string_append(out,"«");
					break;
				case FRENCH:
					g_string_append(out,"»");
					break;
				default:
					g_string_append(out,"\n\\begin_inset Quotes erd\n\\end_inset\n");
				}
			break;
		case NDASH:
		case 0x96:
			g_string_append(out,"--");
			break;
		case MDASH:
		case 0x97:
			g_string_append(out,"---");
			break;
		case ELLIP:
		case 0x85:
			if(scratch->lyx_para_type != GLOSSARYSOURCE){
			  g_string_append(out,"\n\\SpecialChar \\ldots{}\n");
		    } else{
		      g_string_append(out,"...");
		    }
			break;
			default:;
	}
}

/* print_lyx_string - print string, escaping and formatting for LYX */
void print_lyx_string(GString *out, char *str, scratch_pad *scratch, short environment) {
	char *tmp;
	char *start;
	unsigned char extended_character;
	if (str == NULL)
		return;
	start = str;	/* Store start of string */
	if (environment == LYX_PLAIN) {
	   g_string_append(out,"\n\\begin_layout Plain Layout\n\n");
    }   
	while (*str != '\0') {
		/* first look for unicode so it doesn't get caught in the "smart quote" processing */
		/* will use a huristic of looking for a sequence that begins with two bytes of */
		/* the format 11xxxxxx 10xxxxxxxx to indicate a unicode sting */
		/* this is Ok if the second byte is the terminator ('\0') because it is all zeros and won't match */
		if ((((unsigned char)*str & 0xc0) == 0xc0) && ((((unsigned char)*(str+1))  & 0xc0) == 0x80)) { /* hit unicode (huristic */
		   g_string_append_c(out, *str);
		   str++;
		   while ((((unsigned char)*str != '\0')) && (((unsigned char)*str & 0xc0) == 0x80)){
		   	g_string_append_c(out,*str); /* send out the other bytes */
		   	str++;
		   }
		} else {
		switch ((unsigned char)*str) {  /* cast to handle the "smart quotes" outside the ASCII range - they are in there */
			case '\\':
				g_string_append(out,"\n\\backslash\n\n");
				break;
			case '\"':
				  if (environment == LYX_PLAIN){
				  	g_string_append(out,"\"");
				  } else {
                    g_string_append(out,"\n\\begin_inset Quotes erd\n\\end_inset\n");
                  }
				break;
			case '\n':
				 if(environment == LYX_PLAIN) {
					if (*(str+1) == '\0'){ /* skip last new line */
					break;
					}
				    g_string_append(out,"\n\\end_layout\n\n\\begin_layout Plain Layout\n\n");
			    } else {
	  				tmp = str;
					tmp--;
					if ((tmp > start) && (*tmp == ' ')) {
						g_string_append(out,"\n");
					} else {
						g_string_append(out, "\n "); /* add a space */
					}
			    }
				break;
			case '<': /* look for HTML comment LaTeX escape */
			    if ( (environment != LYX_CODE) && (environment != LYX_PLAIN) && (strncmp(str,"<!--",4) == 0)){
			       str+=4; /* move past delimeter */
			       	g_string_append(out, "\n\\begin_layoutPlain Layout\n\\begin_inset ERT\nstatus collapsed\n\n\\begin_layout Plain Layout\n\n");
                    while(strncmp(str,"-->",3) !=0){
                    	switch (*str){
                    	  case '\\':
				            g_string_append(out,"\n\\backslash\n\n");
				            break;
			              case '\"':
                            g_string_append(out,"\n\\begin_inset Quotes erd\n\\end_inset\n\\end_layout\n");
				            break;
                    	  default: 
                    	    g_string_append_c(out,*str);
                    }
                    	str++;
                    }
                    str+=2; /* and past the end delimeter */
				    g_string_append(out,"\n\n\\end_layout\n\\end_inset\n");
			    }
			    else {
			    	g_string_append_c(out, *str);	
			    }
			    break;
			/* handle "smart Quotes" and other "non ASCII" characters */
			case 0x91:
			case 0x92:
			case 0x93:
			case 0x94:
			case 0x96:
			case 0x97:
			case 0x85:
				extended_character = str[0];
				print_lyx_localized_typography(out,extended_character,scratch);
				break;
			default:
				g_string_append_c(out, *str);
			}	
		str++;
	}
}
	if (environment == LYX_PLAIN) {
	   g_string_append(out,"\n\\end_layout\n");
	   }
}

/* print_lyx_url - print url, escaping for LYX */
void print_lyx_url(GString *out, char *str, scratch_pad *scratch) {
	if (str == NULL)
		return;
	while (*str != '\0') {
		switch (*str) {
			case '$': case '%': case '!':
			case '&': case '_': case '#':
				g_string_append_printf(out, "\\%c", *str);
				break;
			case '^':
				g_string_append(out, "\\^{}");
				break;
			default:
				g_string_append_c(out, *str);
			}
		str++;
	}
}

/* lyx_get_table_dimensions - find the dimensions of a table (rows and columns) */
void lyx_get_table_dimensions(node* list, int *rows, int *cols, scratch_pad *scratch){
	int tmp_rows;
	int tmp_cols;
	*rows = 0;
	*cols = 0;
	tmp_rows = 0;
	tmp_cols = 0;
	while (list != NULL) {
		switch (list->key){
			case TABLEHEAD:
			case TABLEBODY:
				lyx_get_table_dimensions(list->children, &tmp_rows, &tmp_cols, scratch);
				*rows += tmp_rows;
				if (tmp_cols > *cols) {*cols = tmp_cols;};
				break;
			case TABLEROW:
				(*rows)++;
				lyx_get_table_dimensions(list->children, &tmp_rows, &tmp_cols,scratch);
				if (tmp_cols>*cols) {*cols = tmp_cols;};
				break;
			case TABLECELL:
				(*cols)++;
			    break;
		}
		list = list->next;
	}
}
/* add_prefixes -- go through node tree and find elements created from headers, figures, and tables
                   add the prefix so LyX can create proper references */
void add_prefixes(node *list, node *root, scratch_pad *scratch) {
	char *label;
	GString *pound_label;
	int  lev;
	
		
	while (list != NULL) {
		switch (list->key) {
			case H1: case H2: case H3: case H4: case H5: case H6:
				lev = list->key - H1 + scratch->baseheaderlevel;
				if (lev > 7)
				lev = 7;	/* Max at level 7 */
				if (list->children->key != AUTOLABEL) {
					label = label_from_node_tree(list->children);
				} else{
					label = label_from_string(list->children->str);
				}
				
					/* update any links in the tree */
				
				pound_label = g_string_new("#");
                g_string_append(pound_label,label);
				update_link_source(pound_label->str,heading_name[lev-1]->str,root);
				
				
				/* and any in the "links" list */
					
				update_links(pound_label->str,heading_name[lev-1]->str,scratch);
				
				g_string_free(pound_label,TRUE);
				free(label);
				break;
			case TABLE:
				if (list->children->key == TABLECAPTION) {
					if (list->children->children->key == TABLELABEL){
					  label = label_from_string(list->children->children->str);
				    } else {
				      label = label_from_node_tree(list->children->children);
				    }
			   		pound_label = g_string_new("#");
                    g_string_append(pound_label,label);
                    update_links(pound_label->str,"tab",scratch);
                    g_string_free(pound_label,TRUE);
					free(label);
					
				}
				break;
			case IMAGE:
			case IMAGEBLOCK:
				if ((list->link_data != NULL) && (list->link_data->label != NULL)) {
                    label = label_from_string(list->link_data->label);
                    pound_label = g_string_new("#");
                    g_string_append(pound_label,label);
                    update_link_source(pound_label->str,"fig",root);
                    g_string_free(pound_label,TRUE);
                    free(label);
				}
				break;
			case HEADINGSECTION:
				add_prefixes(list->children, root, scratch);
				break;
			default:
				break;
		}
		list = list->next;
	}
}
void update_links(char *source,char *prefix, scratch_pad *scratch) {
	node* n = scratch->links;
	link_data *l;
	char* new_source;
	while (n != NULL) {
		l = n->link_data;
			if ((l != NULL) && (l->source != NULL)) {
	   	  		if (strcmp (l->source,source) == 0){
	   	  			new_source = prefix_label(prefix,l->source,TRUE);
					free(l->source);
		    		l->source = strdup(new_source);
					free(new_source);
	   	  		}
		}
		n = n->next;
	}
}

/* update_link_source - walk the tree and add prefixes */
void update_link_source(char *source, char *prefix,node *n){
	 link_data *l;
	 char* new_source;
	 while(n != NULL){
	   	if (n->key == LINK){
	   		l = n->link_data;
	   		if ((l != NULL) && (l->source != NULL)) {
	   	  		if (strcmp (l->source,source) == 0){
	   	  			new_source = prefix_label(prefix,l->source,TRUE);
					free(l->source);
		    		l->source = strdup(new_source);
					free(new_source);
	   	  		}
	   		}
		}
		if(n->children != NULL) {
		   update_link_source(source,prefix,n->children);
		}
	 n = n-> next;	
	 }
	
}

/* prefix_label - Builds a label with a prefix - Returns a null-terminated string,
	which must be freed after use. */
char *prefix_label(char *prefix, char *label, bool pound) {
	char* function_result;
	char short_prefix[6];
	int i;
	strncpy(short_prefix,prefix,5);
	short_prefix[5]= '\0'; /* no terminator if strncpy ends because of length */
	for(i = 0; short_prefix[i]; i++){
  		short_prefix[i] = tolower(short_prefix[i]);
	}
	GString *result = g_string_new("");
	if (label[0]== '#'){
		g_string_append(result,label+1);  /* drop the pound */
	} else{
		g_string_append(result,label);
	}
	g_string_prepend(result,":"); /* prefix colon */
	g_string_prepend(result,short_prefix); /* add the prefix */
	if (pound){
		g_string_prepend(result,"#"); /* add back in the pound if needed */
	}
	function_result = result->str;
	g_string_free(result,FALSE); /* sending back the string */
	return function_result;
}

/* print_escaped_node_tree - print a list of elements as original text */
void print_escaped_node_tree(GString *out, node *n) {
#ifdef DEBUG_ON
	if (n != NULL)
		fprintf(stderr, "start print_escaped_node_tree: '%d'\n",n->key);
#endif
	while (n != NULL) {
		print_escaped_node(out, n);
		n = n->next;
	}
#ifdef DEBUG_ON
	if (n != NULL)
		fprintf(stderr, "finish print_escaped_node_tree: '%d'\n",n->key);
#endif
}

/* print_escaped_node - print an element as original text with LyX escaping*/
void print_escaped_node(GString *out, node *n) {
	  switch (n->key) {
			case STR:
				g_string_append(out,n->str);
				break;
			case SPACE:
				g_string_append(out," ");
				break;
			case APOSTROPHE:
				g_string_append(out,"'");
				break;
			case SINGLEQUOTED:
				g_string_append(out,"'");
				print_escaped_node_tree(out,n->children);
				g_string_append(out,"'");
				return;
			case DOUBLEQUOTED:
				g_string_append(out,"\"");
				print_escaped_node_tree(out,n->children);
				g_string_append(out,"\"");
				return;
			case ELLIPSIS:
				g_string_append(out,"...");
				break;
			case ENDASH:
				g_string_append(out,"--");
				break;
			case EMDASH:
				g_string_append(out,"---");
			default:
				break;
	  }
    	print_escaped_node_tree(out, n->children);
	}
	/* escape string - replace double quotes with escaped version */
char * escape_string(char *str) {
	GString *out = g_string_new("");
	char *clean;
	while (*str != '\0') {
		if (*str == '"') {
				g_string_append(out, "\\\"");
		} else {
			g_string_append_c(out, *str);
		}
		str++;
	}
	clean = out->str;
	g_string_free(out, false);
	return clean;
}
