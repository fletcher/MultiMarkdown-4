/*

	lyxbeamer.c -- Beamer add-on to LyX writer

    (c) 2013 Charles R. Cowan
	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).
	
	Derrived from MultiMarkdown by Fletcher T. Penney - added code to support the LYY format directly

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "lyxbeamer.h"
#include "lyx.h"

bool need_fragile; // if the frame needs to be fragile

/* print_beamer_node_tree -- convert node tree to LyX */
void print_lyxbeamer_node_tree(GString *out, node *list, scratch_pad *scratch, bool no_newline) {
	while (list != NULL) {
		print_lyxbeamer_node(out, list, scratch, no_newline);
		list = list->next;
	}
}

/* print_beamer_node -- convert given node to LyX and append */
void print_lyxbeamer_node(GString *out, node *n, scratch_pad *scratch, bool no_newline) {
	int lev;
	int i;
	char *temp;
	char *prefixed_label;
	node *temp_node;
	int old_type;
	
	/* If we are forcing a complete document, and METADATA isn't the first thing,
		we need to close <head> */
	if ((scratch->extensions & EXT_COMPLETE)
		&& !(scratch->extensions & EXT_HEAD_CLOSED) && 
		!((n->key == FOOTER) || (n->key == METADATA))) {
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
		}
	switch (n->key) {
		case FOOTER:
			if (scratch->lyx_in_frame){  // have an open frame
				      g_string_append(out,"\n\\end_deeper");
//				      g_string_append(out,"\n\\end_layout");
				      g_string_append(out, "\n\\begin_layout Separator");
					  g_string_append(out, "\n\\end_layout");
        	}
        	scratch->lyx_in_frame = FALSE; 
        	print_lyxbeamer_endnotes(out, scratch);
			break;
        case BULLETLIST:
		case ORDEREDLIST:
			scratch -> lyx_beamerbullet = TRUE;
		case DEFLIST:
			old_type = scratch->lyx_para_type;
			scratch->lyx_para_type = n->key;
			scratch->lyx_level++;
			if (scratch->lyx_level > 1){
				g_string_append(out,"\n\\begin_deeper\n");
			}
			print_lyxbeamer_node_tree(out, n->children, scratch, FALSE);
		    scratch->lyx_level--;
		    if (scratch->lyx_level > 0){   
		        g_string_append(out,"\n\\end_deeper\n");
		    }
			scratch->lyx_para_type = old_type;
			if (scratch->lyx_definition_open){
			   g_string_append(out,"\n\\end_deeper\n");
			   scratch->lyx_definition_open = FALSE;
            }
			break;
		case LISTITEM:
			temp_node = n-> children; // should be a list node
			if (temp_node->key != LIST){
				fprintf(stderr,"\nUnanticipated List Item Format");
				exit(EXIT_FAILURE);
			} else {
				temp_node = temp_node-> children; // process the list
				i = 0;
				while (temp_node != NULL){
					i++;
					if (i == 2){
					  g_string_append(out,"\n\\begin_deeper\n");
					  old_type = scratch->lyx_para_type;
					  scratch->lyx_para_type = PARA; // and make it a paragraph, not a list item
					}
					print_lyx_node(out, temp_node, scratch, no_newline);
       		        temp_node = temp_node->next;
					
				}
				if (i>1){
					scratch->lyx_para_type = old_type; // reset the paragraph type
					g_string_append(out,"\n\\end_deeper\n");
				}
			}
			break;
        case HEADINGSECTION:
        	if (scratch->lyx_in_frame){  // have an open frame
				      g_string_append(out,"\n\\end_deeper");
//				      g_string_append(out,"\n\\end_layout");
				      g_string_append(out, "\n\\begin_layout Separator");
					  g_string_append(out, "\n\\end_layout");
        	}
        	scratch->lyx_in_frame = FALSE;
        	need_fragile = FALSE;
        	if (tree_contains_key(n->children,VERBATIM)) {
					need_fragile = TRUE;
				}
			print_lyxbeamer_node_tree(out,n->children,scratch , FALSE);
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;  /* assumes H1 ... H6 are in order */
			switch (lev) {
				case 1:
					g_string_append(out, "\n\\begin_layout Part\n");
					break;
				case 2:
					g_string_append(out, "\n\\begin_layout Section\n");
					break;
				case 3:
					if (need_fragile) {
					  g_string_append(out, "\n\\begin_layout FragileFrame");
				    } else {
					  g_string_append(out, "\n\\begin_layout Frame");
				    };	
					g_string_append(out,"\n\\begin_inset Argument 4");
					g_string_append(out,"\nstatus open\n");
					g_string_append(out,"\n\\begin_layout Plain Layout\n");
				    scratch->lyx_in_frame = TRUE;
					break;
				case 4:
					g_string_append(out,"\n\\begin_layout Standard");
					g_string_append(out, "\n\\begin_inset Flex ArticleMode");
					g_string_append(out, "\nstatus open\n\n");
					g_string_append(out,"\n\\begin_layout Plain Layout\n");
					break;
				default:
					g_string_append(out,"\n\\begin_layout Standard");
					g_string_append(out, "\n\\emph on\n");
					break;
			}
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
			switch(lev){
				case 1: case 2:
				    g_string_append(out,"\n\\end_layout\n");
					break;
				case 3:
					  g_string_append(out,"\n\\end_layout\n");
					  g_string_append(out,"\n\\end_inset\n");
					  g_string_append(out,"\n\\end_layout\n");
				      g_string_append(out,"\n\\begin_deeper\n");
					break;
				case 4:
					g_string_append(out,"\n\\end_layout");
					g_string_append(out,"\n\\end_inset");
					g_string_append(out,"\n\\end_layout");
					break;
				default:
					g_string_append(out, "\n\\emph default\n");
					g_string_append(out,"\n\\end_layout");
					break;
			}
			
			break;
		default:
			/* most things are not changed for beamer output */
			print_lyx_node(out, n, scratch,no_newline);
	}
}

/* print_beamer_endnotes */
void print_lyxbeamer_endnotes(GString *out, scratch_pad *scratch) {
	node *temp_node;
	scratch->used_notes = reverse_list(scratch->used_notes);
	node *note = scratch->used_notes;

    	    // Handle Glossary
    temp_node = note;
    while (temp_node != NULL){
    	if(temp_node->key == GLOSSARYSOURCE){
    	g_string_append(out, "\n\\begin_layout BeginFrame\nGlossary\n");
    	g_string_append(out,"\n\\begin_layout Standard");
    	g_string_append(out,"\n\\begin_inset CommandInset nomencl_print");
    	g_string_append(out,"\nLatexCommand printnomenclature");
    	g_string_append(out,"\nset_width \"auto\"\n");
    	g_string_append(out,"\n\\end_inset\n");
    	g_string_append(out,"\n\\end_layout\n");
    	g_string_append(out, "\n\\end_layout\n");
    	g_string_append(out, "\n\\begin_layout EndFrame");
		g_string_append(out, "\n\\end_layout");
    	break;
        }
		temp_node = temp_node->next;
	}

	if (note == NULL)
		return;
	
	note = scratch->used_notes;
	
	if (tree_contains_key(note,CITATIONSOURCE)){
	   g_string_append(out, "\n\\begin_layout BeginFrame\nReferences\n");
	   g_string_append(out, "\n\\end_layout");
    }
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
	g_string_append(out, "\n\\begin_layout EndFrame"); // close last frame
	g_string_append(out, "\n\\end_layout");

}
