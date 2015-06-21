/*

	odf.c -- ODF (Flat OpenDocument format) writer

	(c) 2013-2015 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "odf.h"

/* begin_odf_output -- handle the initial prefix, if any */
void begin_odf_output(GString *out, node* list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "begin_odf_output\n");
#endif	
	print_odf_header(out);

	if (list == NULL) {
		g_string_append_printf(out, "<office:body>\n<office:text>\n");
	}
}

/* end_odf_output -- close the document */
void end_odf_output(GString *out, node* list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "end_odf_output\n");
#endif
	print_odf_footer(out);
}

/* print_odf_node_tree -- convert node tree to LaTeX */
void print_odf_node_tree(GString *out, node *list, scratch_pad *scratch) {
#ifdef DEBUG_ON
	fprintf(stderr, "print_odf_node_tree\n");
#endif
	while (list != NULL) {
		print_odf_node(out, list, scratch);
		list = list->next;
	}
}

/* print_odf_node -- convert given node to odf and append */
void print_odf_node(GString *out, node *n, scratch_pad *scratch) {
	node *temp_node;
	link_data *temp_link_data = NULL;
	char *temp;
	int lev;
	int old_type;
	char *width = NULL;
	char *height = NULL;
	GString *temp_str;
	int i;

	if (n == NULL)
		return;
	
	/* debugging statement */
#ifdef DEBUG_ON
	fprintf(stderr, "print_odf_node: %d\n",n->key);
#endif

	/* If we are forcing a complete document, and METADATA isn't the first thing,
		we need to close <head> */
	if (!(scratch->extensions & EXT_HEAD_CLOSED) && 
		!((n->key == FOOTER) || (n->key == METADATA))) {
			g_string_append_printf(out, "<office:body>\n<office:text>\n");
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
		}
	
	switch (n->key) {
		case NO_TYPE:
		case ABBREVIATION:
			break;
		case LIST:
			print_odf_node_tree(out,n->children,scratch);
			break;
		case STR:
		case ABBR:
		case ABBRSTART:
		case ABBRSTOP:
			/* TODO: Need something a bit better here for abbreviations */
			print_html_string(out,n->str, scratch);
			break;
		case SPACE:
			g_string_append_printf(out,"%s",n->str);
			break;
		case PLAIN:
			pad(out,1, scratch);
			print_odf_node_tree(out,n->children, scratch);
			scratch->padded = 0;
			break;
		case PARA:
			pad(out, 2, scratch);
			g_string_append_printf(out, "<text:p");
			switch (scratch->odf_para_type) {
				case DEFINITION:
				case BLOCKQUOTE:
					g_string_append_printf(out, " text:style-name=\"Quotations\"");
					break;
				case CODE:
				case VERBATIM:
				case VERBATIMFENCE:
					g_string_append_printf(out, " text:style-name=\"Preformatted Text\"");
					break;
				case ORDEREDLIST:
					g_string_append_printf(out, " text:style-name=\"P2\"");
					break;
				case BULLETLIST:
					g_string_append_printf(out, " text:style-name=\"P1\"");
					break;
				case NOTEREFERENCE:
				case NOTESOURCE:
				case CITATION:
				case NOCITATION:
					g_string_append_printf(out, " text:style-name=\"Footnote\"");
					break;
				default:
					g_string_append_printf(out, " text:style-name=\"Standard\"");
					break;
			}
			g_string_append_printf(out, ">");
			print_odf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "</text:p>\n");
			scratch->padded = 1;
			break;
		case HRULE:
			pad(out, 2, scratch);
			g_string_append_printf(out,"<text:p text:style-name=\"Horizontal_20_Line\"/>");
			scratch->padded = 0;
			break;
		case HTMLBLOCK:
			/* don't print HTML block */
			/* but do print HTML comments for raw LaTeX */
			if (strncmp(n->str,"<!--",4) == 0) {
				pad(out, 2, scratch);
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "<text:p text:style-name=\"Standard\">%s</text:p>", &n->str[4]);
				scratch->padded = 0;
			}
			break;
		case VERBATIM:
		case VERBATIMFENCE:
			old_type = scratch->odf_para_type;
			scratch->odf_para_type = VERBATIM;
			pad(out, 2, scratch);
			g_string_append_printf(out, "<text:p text:style-name=\"Preformatted Text\">");
			print_odf_code_string(out, n->str);
			g_string_append_printf(out, "</text:p>\n");
 			scratch->padded = 0;
			scratch->odf_para_type = old_type;
			break;
		case BULLETLIST:
		case ORDEREDLIST:
			old_type = scratch->odf_para_type;
			scratch->odf_para_type = n->key;
			if (scratch->odf_list_needs_end_p) {
				g_string_append_printf(out, "</text:p>");
				scratch->odf_list_needs_end_p = false;
			}
			pad(out, 2, scratch);
			switch (n->key) {
				case BULLETLIST:
					g_string_append_printf(out, "<text:list text:style-name=\"L1\">");
					break;
				case ORDEREDLIST:
					g_string_append_printf(out, "<text:list text:style-name=\"L2\">");
					break;
			}
			scratch->padded = 1;
			print_odf_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "</text:list>");
			scratch->padded = 0;
			scratch->odf_para_type = old_type;
			break;
		case LISTITEM:
#ifdef DEBUG_ON
	fprintf(stderr, "print list item\n");
#endif
			pad(out, 1, scratch);
			g_string_append_printf(out, "<text:list-item>\n");
			if ((n->children != NULL) && (n->children->children != NULL) && (n->children->children->key != PARA)) {
				switch (scratch->odf_para_type) {
					case BULLETLIST:
						g_string_append_printf(out, "<text:p text:style-name=\"P1\">");
						break;
					case ORDEREDLIST:
						g_string_append_printf(out, "<text:p text:style-name=\"P2\">");
						break;
				}
				scratch->odf_list_needs_end_p = true;
			} else if ((n->children != NULL) && (n->children->key == LIST) && (n->children->children == NULL)) {
				/* This is an empty list item.  ODF apparently requires something for the empty item to appear */
				switch (scratch->odf_para_type) {
					case BULLETLIST:
						g_string_append_printf(out, "<text:p text:style-name=\"P1\"/>");
						break;
					case ORDEREDLIST:
						g_string_append_printf(out, "<text:p text:style-name=\"P2\"/>");
						break;
				}
			}
			scratch->padded = 2;
			print_odf_node_tree(out, n->children, scratch);
			scratch->odf_list_needs_end_p = false;
#ifdef DEBUG_ON
	fprintf(stderr, "print list tree\n");
#endif
			if (!(tree_contains_key(n->children, BULLETLIST)) && 
				!(tree_contains_key(n->children, ORDEREDLIST))) {
				if ((n->children != NULL) && (n->children->children != NULL) && (n->children->children->key != PARA))
					g_string_append_printf(out, "</text:p>");
			}
			g_string_append_printf(out, "</text:list-item>\n");
			scratch->padded = 1;
#ifdef DEBUG_ON
	fprintf(stderr, "finish print list item\n");
#endif
			break;
		case METADATA:
			g_string_append_printf(out, "<office:meta>\n");
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
			print_odf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</office:meta>\n");
			temp_node = metadata_for_key("odfheader", n);
			if (temp_node != NULL) {
				print_raw_node(out, temp_node->children);
			}
			g_string_append_printf(out, "<office:body>\n<office:text>\n");
			break;
		case METAKEY:
			temp = label_from_string(n->str);
			if (strcmp(temp, "title") == 0) {
				g_string_append_printf(out, "<dc:title>");
				print_odf_node(out, n->children, scratch);
				g_string_append_printf(out, "</dc:title>\n");
			} else if (strcmp(temp, "css") == 0) {
			} else if (strcmp(temp, "xhtmlheader") == 0) {
			} else if (strcmp(temp, "htmlheader") == 0) {
			} else if (strcmp(n->str, "mmdfooter") == 0) {
			} else if (strcmp(n->str, "mmdheader") == 0) {
			} else if (strcmp(temp, "baseheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
			} else if (strcmp(temp, "odfheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
			} else if (strcmp(temp, "htmlheaderlevel") == 0) {
			} else if (strcmp(temp, "latexinput") == 0) {
			} else if (strcmp(temp, "latexfooter") == 0) {
			} else if (strcmp(temp, "latexmode") == 0) {
			} else if (strcmp(temp, "keywords") == 0) {
				g_string_append_printf(out, "<meta:keyword>");
				print_odf_node(out, n->children, scratch);
				g_string_append_printf(out, "</meta:keyword>\n");
			} else if (strcmp(temp, "quoteslanguage") == 0) {
				free(temp);
				temp = label_from_node_tree(n->children);
				if ((strcmp(temp, "nl") == 0) || (strcmp(temp, "dutch") == 0)) { scratch->language = DUTCH; }   else 
				if ((strcmp(temp, "de") == 0) || (strcmp(temp, "german") == 0)) { scratch->language = GERMAN; } else 
				if (strcmp(temp, "germanguillemets") == 0) { scratch->language = GERMANGUILL; } else 
				if ((strcmp(temp, "fr") == 0) || (strcmp(temp, "french") == 0)) { scratch->language = FRENCH; } else 
				if ((strcmp(temp, "sv") == 0) || (strcmp(temp, "swedish") == 0)) { scratch->language = SWEDISH; }
			} else if (strcmp(temp, "lang") == 0) {
			} else {
				g_string_append_printf(out,"<meta:user-defined meta:name=\"");
				print_odf_string(out, n->str);
				g_string_append_printf(out, "\">");
				print_odf_node(out,n->children,scratch);
				g_string_append_printf(out,"</meta:user-defined>\n");
			}
			free(temp);
			break;
		case METAVALUE:
			trim_trailing_whitespace(n->str);
			print_odf_string(out,n->str);
			break;
		case FOOTER:
			break;
		case HEADINGSECTION:
			print_odf_node_tree(out,n->children,scratch);
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;  /* assumes H1 ... H6 are in order */
			pad(out, 2, scratch);
			g_string_append_printf(out, "<text:h text:outline-level=\"%d\">", lev);
			if (n->children->key == AUTOLABEL) {
				/* use label for header since one was specified (MMD)*/
				temp = label_from_string(n->children->str);
				g_string_append_printf(out, "<text:bookmark text:name=\"%s\"/>", temp);
				print_odf_node_tree(out, n->children->next, scratch);
				g_string_append_printf(out, "<text:bookmark-end text:name=\"%s\"/>", temp);
				free(temp);
			} else {
				/* generate a label by default for MMD */
				temp = label_from_node_tree(n->children);
				g_string_append_printf(out, "<text:bookmark text:name=\"%s\"/>", temp);
				print_odf_node_tree(out, n->children, scratch);
				g_string_append_printf(out, "<text:bookmark-end text:name=\"%s\"/>", temp);
				free(temp);
			}
			g_string_append_printf(out, "</text:h>");
			scratch->padded = 0;
			break;
		case APOSTROPHE:
			print_html_localized_typography(out, APOS, scratch);
			break;
		case ELLIPSIS:
			print_html_localized_typography(out, ELLIP, scratch);
			break;
		case EMDASH:
			print_html_localized_typography(out, MDASH, scratch);
			break;
		case ENDASH:
			print_html_localized_typography(out, NDASH, scratch);
			break;
		case SINGLEQUOTED:
			print_html_localized_typography(out, LSQUOTE, scratch);
			print_odf_node_tree(out, n->children, scratch);
			print_html_localized_typography(out, RSQUOTE, scratch);
			break;
		case DOUBLEQUOTED:
			print_html_localized_typography(out, LDQUOTE, scratch);
			print_odf_node_tree(out, n->children, scratch);
			print_html_localized_typography(out, RDQUOTE, scratch);
			break;
		case LINEBREAK:
			g_string_append_printf(out, "<text:line-break/>");
			break;
		case MATHSPAN:
			temp = strdup(n->str);
			if (temp[0] == '$') {
				g_string_append_printf(out, "<text:span text:style-name=\"math\">%s</text:span>",temp);
			} else if (temp[strlen(temp) - 1] == ']') {
				temp[strlen(temp) - 3] = '\0';
				g_string_append_printf(out, "<text:span text:style-name=\"math\">%s\\]</text:span>",temp);
			} else {
				temp[strlen(temp) - 3] = '\0';
				g_string_append_printf(out, "<text:span text:style-name=\"math\">%s\\)</text:span>",temp);
			}
			free(temp);
			break;
		case STRONG:
			g_string_append_printf(out, "<text:span text:style-name=\"MMD-Bold\">");
			print_odf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "</text:span>");
			break;
		case EMPH:
			g_string_append_printf(out, "<text:span text:style-name=\"MMD-Italic\">");
			print_odf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "</text:span>");
			break;
		case LINKREFERENCE:
			break;
		case LINK:
#ifdef DEBUG_ON
	fprintf(stderr, "print odf link: '%s'\n",n->str);
#endif
			/* Stash a copy of the link data */
			if (n->link_data != NULL)
				temp_link_data = mk_link_data(n->link_data->label, n->link_data->source, n->link_data->title, n->link_data->attr);

			/* Do we have proper info? */
			if (n->link_data == NULL) {
				/* NULL link_data could occur if we parsed this link before and it didn't
					match anything */
				n->link_data =  mk_link_data(NULL, NULL, NULL, NULL);
			}
			
			if ((n->link_data->label == NULL) &&
			(n->link_data->source == NULL)) {
#ifdef DEBUG_ON
	fprintf(stderr, "print odf link: '%s'\n",n->str);
#endif
				/* we seem to be a [foo][] style link */
				/* so load a label */
				temp_str = g_string_new("");
				print_raw_node_tree(temp_str, n->children);
				free(n->link_data->label);
				n->link_data->label = temp_str->str;
				g_string_free(temp_str, FALSE);
			}
#ifdef DEBUG_ON
	fprintf(stderr, "look for reference data for odf link: '%s'\n",n->str);
#endif
			/* Load reference data */
			if (n->link_data->label != NULL) {
#ifdef DEBUG_ON
	fprintf(stderr, "have label for odf link: '%s'\n",n->str);
#endif
				temp = strdup(n->link_data->label);
				free_link_data(n->link_data);
				n->link_data = extract_link_data(temp, scratch);
				if (n->link_data == NULL) {
					/* replace original text since no definition found */
					g_string_append_printf(out, "[");
					print_odf_node(out, n->children, scratch);
					g_string_append_printf(out,"]");
					if (n->children->next != NULL) {
						g_string_append_printf(out, "[");
						print_odf_node_tree(out, n->children->next, scratch);
						g_string_append_printf(out,"]");
					} else if (n->str != NULL) {
						/* no title label, so see if we stashed str*/
						g_string_append_printf(out, "%s", n->str);
					} else {
						g_string_append_printf(out, "[%s]",temp);
					}

					free(temp);

					/* Restore stashed copy */
					n->link_data = temp_link_data;

					break;
				}
				free(temp);
			}
#ifdef DEBUG_ON
	fprintf(stderr, "got link data for odf link: '%s'\n",n->str);
#endif
			g_string_append_printf(out, "<text:a xlink:type=\"simple\"");
			if (n->link_data->source != NULL) {
				g_string_append_printf(out, " xlink:href=\"");
				print_html_string(out,n->link_data->source, scratch);
				g_string_append_printf(out, "\"");
			}
			if ((n->link_data->title != NULL) && (strlen(n->link_data->title) > 0)) {
				g_string_append_printf(out, " office:name=\"");
				print_html_string(out, n->link_data->title, scratch);
				g_string_append_printf(out, "\"");
			}
			print_odf_node_tree(out, n->link_data->attr, scratch);
			g_string_append_printf(out, ">");
			if (n->children != NULL)
				print_odf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "</text:a>");

			/* Restore stashed copy */
			n->link_data->attr = NULL;
			free_link_data(n->link_data);
			n->link_data = temp_link_data;

			break;
		case ATTRKEY:
			if ( (strcmp(n->str,"height") == 0) || (strcmp(n->str, "width") == 0)) {
			}
			break;
		case REFNAME:
		case SOURCE:
		case TITLE:
			break;
		case IMAGEBLOCK:
			pad(out,2, scratch);
		case IMAGE:
#ifdef DEBUG_ON
	fprintf(stderr, "print image\n");
#endif
			/* Stash a copy of the link data */
			if (n->link_data != NULL)
				temp_link_data = mk_link_data(n->link_data->label, n->link_data->source, n->link_data->title, n->link_data->attr);

			if (n->key == IMAGEBLOCK)
				g_string_append_printf(out, "<text:p>\n");
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
#ifdef DEBUG_ON
	fprintf(stderr, "load reference data\n");
#endif
			/* Load reference data */
			if (n->link_data->label != NULL) {
				temp = strdup(n->link_data->label);
				free_link_data(n->link_data);
				n->link_data = extract_link_data(temp, scratch);
				if (n->link_data == NULL) {
					g_string_append_printf(out, "![");
					print_html_node_tree(out, n->children, scratch);
					g_string_append_printf(out,"][%s]",temp);

					/* Restore stashed copy */
					n->link_data = temp_link_data;

					free(temp);
					
					break;
				}
				free(temp);
			}
#ifdef DEBUG_ON
	fprintf(stderr, "create img\n");
#endif
			g_string_append_printf(out, "<draw:frame text:anchor-type=\"as-char\"\ndraw:z-index=\"0\" draw:style-name=\"fr1\" ");

			if (n->link_data->attr != NULL) {
				temp_node = node_for_attribute("height",n->link_data->attr);
				if (temp_node != NULL)
					height = correct_dimension_units(temp_node->children->str);
				temp_node = node_for_attribute("width",n->link_data->attr);
				if (temp_node != NULL)
					width = correct_dimension_units(temp_node->children->str);
			}
			
			if (width != NULL) {
				g_string_append_printf(out, "svg:width=\"%s\"\n", width);
			} else {
				g_string_append_printf(out, "svg:width=\"95%%\"\n");
			}
			
			g_string_append_printf(out, ">\n<draw:text-box><text:p><draw:frame text:anchor-type=\"as-char\" draw:z-index=\"1\" ");
			if ((height != NULL) && (width != NULL)) {
				g_string_append_printf(out, "svg:height=\"%s\"\n",height);
				g_string_append_printf(out, "svg:width=\"%s\"\n", width);
			}
			
			if (n->link_data->source != NULL)
				g_string_append_printf(out, "><draw:image xlink:href=\"%s\"",n->link_data->source);

			g_string_append_printf(out," xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\" draw:filter-name=\"&lt;All formats&gt;\"/>\n</draw:frame></text:p>");

			if (n->key == IMAGEBLOCK) {
				if (n->children != NULL) {
					temp_str = g_string_new("");
					print_odf_node_tree(temp_str,n->children,scratch);
					if (temp_str->currentStringLength > 0) {
					g_string_append_printf(out, "<text:p>Figure <text:sequence text:name=\"Figure\" text:formula=\"ooow:Figure+1\" style:num-format=\"1\"> Update Fields to calculate numbers</text:sequence>: ");
						g_string_append(out, temp_str->str);
						g_string_append_printf(out, "</text:p>");
					}
					g_string_free(temp_str, true);
				}
				g_string_append_printf(out, "</draw:text-box></draw:frame>\n</text:p>\n");
			} else {
				g_string_append_printf(out, "</draw:text-box></draw:frame>\n");
			}
			scratch->padded = 1;

			/* Restore stashed copy */
			n->link_data->attr = NULL;
			free_link_data(n->link_data);
			n->link_data = temp_link_data;
			
			free(height);
			free(width);
			
			break;
#ifdef DEBUG_ON
	fprintf(stderr, "finish image\n");
#endif
		case NOTEREFERENCE:
			old_type = scratch->odf_para_type;
			scratch->odf_para_type = NOTEREFERENCE;
			lev = note_number_for_node(n, scratch);
			temp_node = node_for_count(scratch->used_notes, lev);
			scratch->padded = 2;
			scratch->printing_notes = 1;
			if (temp_node->key == GLOSSARYSOURCE) {
				g_string_append_printf(out, "<text:note text:id=\"\" text:note-class=\"glossary\"><text:note-body>\n");
				print_odf_node_tree(out, temp_node->children, scratch);
				g_string_append_printf(out, "</text:note-body>\n</text:note>\n");
			} else {
				g_string_append_printf(out, "<text:note text:id=\"\" text:note-class=\"footnote\"><text:note-body>\n");
				print_odf_node_tree(out, temp_node->children, scratch);
				g_string_append_printf(out, "</text:note-body>\n</text:note>\n");
			}
			scratch->printing_notes = 0;
			scratch->padded = 1;
			scratch->odf_para_type = old_type;
			break;
		case NOCITATION:
		case CITATION:
#ifdef DEBUG_ON
	fprintf(stderr, "print cite\n");
#endif
			if ((n->link_data != NULL) && (strncmp(n->link_data->label,"[#",2) == 0)) {
				/* external citation */
#ifdef DEBUG_ON
					fprintf(stderr, "external first??");
#endif
				g_string_append_printf(out, "%s", n->link_data->label);
			} else {
#ifdef DEBUG_ON
				fprintf(stderr, "internal cite\n");
#endif
				/* MMD citation, so output as footnote */
				/* TODO: create separate stream from footnotes */
				scratch->printing_notes = 1;
				lev = 0;
				if (n->link_data != NULL)
					lev = note_number_for_label(n->link_data->label, scratch);
				if (lev != 0) {
#ifdef DEBUG_ON
					fprintf(stderr, "matching cite found - %d\n", lev);
#endif
					temp_node = node_for_count(scratch->used_notes, lev);
					/* flag that this is used as a citation */
					temp_node->key = CITATIONSOURCE;
					if (lev > scratch->max_footnote_num) {
						/* first use of this citation */
						scratch->max_footnote_num = lev;
						
						old_type = scratch->odf_para_type;
						scratch->odf_para_type = CITATION;
						
						/* change to represent cite count only */
						lev = cite_count_node_from_end(temp_node);
						g_string_append_printf(out, "<text:note text:id=\"cite%d\" text:note-class=\"endnote\"><text:note-body>\n", lev);
						scratch->padded = 2;
						if (temp_node->children != NULL) {
							print_odf_node(out, temp_node->children, scratch);
						}
						pad(out, 1, scratch);
						g_string_append_printf(out, "</text:note-body>\n</text:note>\n");
						scratch->odf_para_type = old_type;
					} else {
						/* We are reusing a previous citation */
#ifdef DEBUG_ON
					fprintf(stderr, "link to existing cite %d\n", lev);
#endif
						/* Change lev to represent cite count only */
						lev = cite_count_node_from_end(temp_node);
#ifdef DEBUG_ON
					fprintf(stderr, "renumbered to %d\n", lev);
#endif
					
						g_string_append_printf(out, "<text:span text:style-name=\"Footnote_20_anchor\"><text:note-ref text:note-class=\"endnote\" text:reference-format=\"text\" text:ref-name=\"cite%d\">%d</text:note-ref></text:span>", lev, lev);
					}
				} else {
					/* not located -- this is external cite */
#ifdef DEBUG_ON
				fprintf(stderr, "no match for cite: '%s'\n",n->link_data->label);
#endif
					if ((n->link_data != NULL) && (n->key == NOCITATION)) {
						g_string_append_printf(out, "%s", n->link_data->label);
					} else if (n->link_data != NULL) {
						g_string_append_printf(out, "[");
						if (n->children != NULL) {
							print_odf_node(out, n->children, scratch);
							g_string_append_printf(out, "][");
						}
						g_string_append_printf(out, "#%s]",n->link_data->label);
					}
				}
			}
			scratch->printing_notes = 0;
			if ((n->next != NULL) && (n->next->key == CITATION)) {
				g_string_append_printf(out, " ");
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
				print_odf_string(out, temp);
				free(temp);
			}
			break;
		case GLOSSARYTERM:
			g_string_append_printf(out,"<text:p text:style-name=\"Glossary\">");
			print_odf_string(out, n->children->str);
			g_string_append_printf(out, ":</text:p>\n");
			break;
		case GLOSSARYSORTKEY:
			break;
		case CODE:
			g_string_append_printf(out, "<text:span text:style-name=\"Source_20_Text\">");
			print_html_string(out, n->str, scratch);
			g_string_append_printf(out, "</text:span>");
			break;
		case BLOCKQUOTEMARKER:
			print_odf_node_tree(out, n->children, scratch);
			break;
		case BLOCKQUOTE:
			pad(out,2, scratch);
			scratch->padded = 2;
			old_type = scratch->odf_para_type;
			scratch->odf_para_type = BLOCKQUOTE;
			print_odf_node_tree(out, n->children, scratch);
			scratch->odf_para_type = old_type;
			break;
		case RAW:
		/* shouldn't happen */
			assert(n->key != RAW);
			break;
		case HTML:
			/* don't print HTML */
			/* but do print HTML comments for raw ODF */
			if (strncmp(n->str,"<!--",4) == 0) {
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "%s", &n->str[4]);
			}
			break;
		case DEFLIST:
			print_odf_node_tree(out, n->children, scratch);
			break;
		case TERM:
			pad(out,1, scratch);
			g_string_append_printf(out, "<text:p><text:span text:style-name=\"MMD-Bold\">");
			print_odf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</text:span></text:p>\n");
			scratch->padded = 1;
			break;
		case DEFINITION:
			old_type = scratch->odf_para_type;
			scratch->odf_para_type = DEFINITION;
			pad(out,1, scratch);
			scratch->padded = 1;
			g_string_append_printf(out, "<text:p text:style-name=\"Quotations\">");
			print_odf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</text:p>\n");
			scratch->padded = 0;
			scratch->odf_para_type = old_type;
			break;
		case TABLE:
			pad(out,2, scratch);
			g_string_append_printf(out, "<table:table>\n");
			print_odf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</table:table>\n");
			/* caption if present */
			if ((n->children != NULL) && (n->children->key == TABLECAPTION)) {
				if (n->children->children->key == TABLELABEL) {
					temp = label_from_string(n->children->children->str);
				} else {
					temp = label_from_node_tree(n->children->children);
				}
				g_string_append_printf(out,"<text:p><text:bookmark text:name=\"%s\"/>Table <text:sequence text:name=\"Table\" text:formula=\"ooow:Table+1\" style:num-format=\"1\"> Update Fields to calculate numbers</text:sequence>:", temp);
				print_odf_node_tree(out,n->children->children, scratch);
				g_string_append_printf(out, "<text:bookmark-end text:name=\"%s\"/></text:p>\n",temp);
				free(temp);
			}
			scratch->padded = 1;
			scratch->table_alignment = NULL;
			break;
		case TABLESEPARATOR:
			scratch->table_alignment = n->str;
			break;
		case TABLECAPTION:
			break;
		case TABLELABEL:
			break;
		case TABLEHEAD:
			for (i=0; i < strlen(scratch->table_alignment); i++) {
				g_string_append_printf(out, "<table:table-column/>\n");
			}
			scratch->cell_type = 'h';
			print_odf_node_tree(out, n->children, scratch);
			scratch->cell_type = 'd';
			break;
		case TABLEBODY:
			print_odf_node_tree(out, n->children, scratch);
			break;
		case TABLEROW:
			g_string_append_printf(out, "<table:table-row>\n");
			scratch->table_column = 0;
			print_odf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</table:table-row>\n");
			break;
		case TABLECELL:
			temp = scratch->table_alignment;
			if (strncmp(&temp[scratch->table_column],"h",1) == 0) {
				scratch->table_column++;
			}
			lev = scratch->table_column;
			g_string_append_printf(out, "<table:table-cell");
			if ((n->children != NULL) && (n->children->key == CELLSPAN)) {
				g_string_append_printf(out, " table:number-columns-spanned=\"%d\"", strlen(n->children->str)+1);
				scratch->table_column += (int)strlen(n->children->str);
			}
			g_string_append_printf(out, ">\n<text:p");
			if (scratch->cell_type == 'h') {
				g_string_append_printf(out, " text:style-name=\"Table_20_Heading\"");
			} else {
				if ( strncmp(&temp[lev],"r",1) == 0) {
					g_string_append_printf(out, " text:style-name=\"MMD-Table-Right\"");
				} else if ( strncmp(&temp[lev],"R",1) == 0) {
					g_string_append_printf(out, " text:style-name=\"MMD-Table-Right\"");
				} else if ( strncmp(&temp[lev],"c",1) == 0) {
					g_string_append_printf(out, " text:style-name=\"MMD-Table-Center\"");
				} else if ( strncmp(&temp[lev],"C",1) == 0) {
					g_string_append_printf(out, " text:style-name=\"MMD-Table-Center\"");
				} else {
					g_string_append_printf(out, " text:style-name=\"MMD-Table\"");
				}
			}

			g_string_append_printf(out, ">");
			scratch->padded = 2;
			print_odf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</text:p>\n</table:table-cell>\n", scratch->cell_type);
			scratch->table_column++;
			break;
		case CELLSPAN:
			break;
		case GLOSSARYSOURCE:
			if (scratch->printing_notes)
				print_odf_node_tree(out, n->children, scratch);
			break;
		case CITATIONSOURCE:
		case NOTESOURCE:
			if (scratch->printing_notes)
				print_odf_node(out, n->children, scratch);
			break;
		case SOURCEBRANCH:
			fprintf(stderr,"SOURCEBRANCH\n");
			break;
		case NOTELABEL:
		case GLOSSARYLABEL:
			break;
		case SUPERSCRIPT:
			g_string_append_printf(out, "<text:span text:style-name=\"MMD-Superscript\">");
			print_html_string(out,n->str, scratch);
			g_string_append_printf(out, "</text:span>");
			break;
		case SUBSCRIPT:
			g_string_append_printf(out, "<text:span text:style-name=\"MMD-Subscript\">");
			print_html_string(out,n->str, scratch);
			g_string_append_printf(out, "</text:span>");
			break;
		case KEY_COUNTER:
			break;
		case TOC:
			print_odf_node_tree(out,n->children, scratch);
			break;
		default:
			fprintf(stderr, "print_odf_node encountered unknown node key = %d\n",n->key);
			exit(EXIT_FAILURE);
	}
	
#ifdef DEBUG_ON
	fprintf(stderr, "finish print_odf_node: %d\n",n->key);
#endif

}

/* print_odf_string - print string, escaping for odf */
void print_odf_string(GString *out, char *str) {
	char *tmp;
	while (*str != '\0') {
		switch (*str) {
			case '&':
				g_string_append_printf(out, "&amp;");
				break;
			case '<':
				g_string_append_printf(out, "&lt;");
				break;
			case '>':
				g_string_append_printf(out, "&gt;");
				break;
			case '"':
				g_string_append_printf(out, "&quot;");
				break;
			case '\n': case '\r':
				tmp = str;
				tmp--;
				if (*tmp == ' ') {
					tmp--;
					if (*tmp == ' ') {
						g_string_append_printf(out, "<text:line-break/>");
					} else {
						g_string_append_printf(out, "\n");
					}
				} else {
					g_string_append_printf(out, "\n");
				}
				break;
			case ' ':
				tmp = str;
				tmp++;
				if (*tmp == ' ') {
					tmp++;
					if (*tmp == ' ') {
						tmp++;
						if (*tmp == ' ') {
							g_string_append_printf(out, "<text:tab/>");
							str = tmp;
						} else {
							g_string_append_printf(out, " ");
						}
					} else {
						g_string_append_printf(out, " ");
					}
				} else {
					g_string_append_printf(out, " ");
				}
				break;
			default:
				g_string_append_c(out, *str);
		}
		str++;
	}
}

/* print_odf_code_string - print string, escaping for HTML and saving newlines 
*/
void print_odf_code_string(GString *out, char *str) {
	char *tmp;
	while (*str != '\0') {
		switch (*str) {
			case '&':
				g_string_append_printf(out, "&amp;");
				break;
			case '<':
				g_string_append_printf(out, "&lt;");
				break;
			case '>':
				g_string_append_printf(out, "&gt;");
				break;
			case '"':
				g_string_append_printf(out, "&quot;");
				break;
			case '\n':
				g_string_append_printf(out, "<text:line-break/>");
				break;
			case ' ':
				tmp = str;
				tmp++;
				if (*tmp == ' ') {
					tmp++;
					if (*tmp == ' ') {
						tmp++;
						if (*tmp == ' ') {
							g_string_append_printf(out, "<text:tab/>");
							str = tmp;
						} else {
							g_string_append_printf(out, " ");
						}
					} else {
						g_string_append_printf(out, " ");
					}
				} else {
					g_string_append_printf(out, " ");
				}
				break;
			default:
				g_string_append_c(out, *str);
		}
		str++;
	}
}

void print_odf_header(GString *out){
	/* Insert required XML header */
	g_string_append_printf(out,
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
"<office:document xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n" \
"     xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n" \
"     xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n" \
"     xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n" \
"     xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n" \
"     xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n" \
"     xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" \
"     xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n" \
"     xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n" \
"     xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n" \
"     xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n" \
"     xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n" \
"     xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n" \
"     xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n" \
"     xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n" \
"     xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n" \
"     xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\"\n" \
"     xmlns:ooo=\"http://openoffice.org/2004/office\"\n" \
"     xmlns:ooow=\"http://openoffice.org/2004/writer\"\n" \
"     xmlns:oooc=\"http://openoffice.org/2004/calc\"\n" \
"     xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n" \
"     xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n" \
"     xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n" \
"     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n" \
"     xmlns:rpt=\"http://openoffice.org/2005/report\"\n" \
"     xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\"\n" \
"     xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"\n" \
"     xmlns:grddl=\"http://www.w3.org/2003/g/data-view#\"\n" \
"     xmlns:tableooo=\"http://openoffice.org/2009/table\"\n" \
"     xmlns:field=\"urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0\"\n" \
"     xmlns:formx=\"urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0\"\n" \
"     xmlns:css3t=\"http://www.w3.org/TR/css3-text/\"\n" \
"     office:version=\"1.2\"\n" \
"     grddl:transformation=\"http://docs.oasis-open.org/office/1.2/xslt/odf2rdf.xsl\"\n" \
"     office:mimetype=\"application/vnd.oasis.opendocument.text\">\n");
    
	/* Font Declarations */
	g_string_append_printf(out, "<office:font-face-decls>\n" \
	"   <style:font-face style:name=\"Courier New\" svg:font-family=\"'Courier New'\"\n" \
    "                    style:font-adornments=\"Regular\"\n" \
    "                    style:font-family-generic=\"modern\"\n" \
    "                    style:font-pitch=\"fixed\"/>\n" \
    "</office:font-face-decls>\n");
    
    /* Append basic style information */
    g_string_append_printf(out, "<office:styles>\n" \
    "<style:style style:name=\"Standard\" style:family=\"paragraph\" style:class=\"text\">\n" \
    "      <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.15in\"" \
    "     fo:text-align=\"justify\" style:justify-single-word=\"false\"/>\n" \
    "   </style:style>\n" \
    "<style:style style:name=\"Preformatted_20_Text\" style:display-name=\"Preformatted Text\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0in\" fo:text-align=\"start\"\n" \
    "                               style:justify-single-word=\"false\"/>\n" \
    "   <style:text-properties style:font-name=\"Courier New\" fo:font-size=\"11pt\"\n" \
    "                          style:font-name-asian=\"Courier New\"\n" \
    "                          style:font-size-asian=\"11pt\"\n" \
    "                          style:font-name-complex=\"Courier New\"\n" \
    "                          style:font-size-complex=\"11pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Source_20_Text\" style:display-name=\"Source Text\"\n" \
    "             style:family=\"text\">\n" \
    "   <style:text-properties style:font-name=\"Courier New\" style:font-name-asian=\"Courier New\"\n" \
    "                          style:font-name-complex=\"Courier New\"\n" \
    "                          fo:font-size=\"11pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"List\" style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"list\">\n" \
    "   <style:paragraph-properties fo:text-align=\"start\" style:justify-single-word=\"false\"/>\n" \
    "   <style:text-properties style:font-size-asian=\"12pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Quotations\" style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-left=\"0.3937in\" fo:margin-right=\"0.3937in\" fo:margin-top=\"0in\"\n" \
    "                               fo:margin-bottom=\"0.1965in\"\n" \
    "                               fo:text-align=\"justify\"" \
    "                               style:justify-single-word=\"false\"" \
    "                               fo:text-indent=\"0in\"\n" \
    "                               style:auto-text-indent=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Table_20_Heading\" style:display-name=\"Table Heading\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Table_20_Contents\"\n" \
    "             style:class=\"extra\">\n" \
    "   <style:paragraph-properties fo:text-align=\"center\" style:justify-single-word=\"false\"\n" \
    "                               text:number-lines=\"false\"\n" \
    "                               text:line-number=\"0\"/>\n" \
    "   <style:text-properties fo:font-weight=\"bold\" style:font-weight-asian=\"bold\"\n" \
    "                          style:font-weight-complex=\"bold\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Horizontal_20_Line\" style:display-name=\"Horizontal Line\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.1965in\"\n" \
    "                               style:border-line-width-bottom=\"0.0008in 0.0138in 0.0008in\"\n" \
    "                               fo:padding=\"0in\"\n" \
    "                               fo:border-left=\"none\"\n" \
    "                               fo:border-right=\"none\"\n" \
    "                               fo:border-top=\"none\"\n" \
    "                               fo:border-bottom=\"0.0154in double #808080\"\n" \
    "                               text:number-lines=\"false\"\n" \
    "                               text:line-number=\"0\"\n" \
    "                               style:join-border=\"false\"/>\n" \
    "   <style:text-properties fo:font-size=\"6pt\" style:font-size-asian=\"6pt\" style:font-size-complex=\"6pt\"/>\n" \
    "</style:style>\n" \
	"<style:style style:name=\"Footnote_20_anchor\" style:display-name=\"Footnote anchor\"" \
	"              style:family=\"text\">" \
	"    <style:text-properties style:text-position=\"super 58%%\"/>" \
	" </style:style>\n" \
	"  <text:notes-configuration text:note-class=\"footnote\" text:default-style-name=\"Footnote\" text:citation-style-name=\"Footnote_20_Symbol\" text:citation-body-style-name=\"Footnote_20_anchor\" text:master-page-name=\"Footnote\" style:num-format=\"a\" text:start-value=\"0\" text:footnotes-position=\"page\" text:start-numbering-at=\"page\"/>\n" \
	"  <text:notes-configuration text:note-class=\"endnote\" text:default-style-name=\"Endnote\" text:citation-style-name=\"Endnote_20_Symbol\" text:citation-body-style-name=\"Endnote_20_anchor\" text:master-page-name=\"Endnote\" style:num-format=\"1\" text:start-value=\"0\"/>\n" \
    "</office:styles>\n");

    /* Automatic style information */
    g_string_append_printf(out, "<office:automatic-styles>" \
    "   <style:style style:name=\"MMD-Italic\" style:family=\"text\">\n" \
    "      <style:text-properties fo:font-style=\"italic\" style:font-style-asian=\"italic\"\n" \
    "                             style:font-style-complex=\"italic\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Bold\" style:family=\"text\">\n" \
    "      <style:text-properties fo:font-weight=\"bold\" style:font-weight-asian=\"bold\"\n" \
    "                             style:font-weight-complex=\"bold\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Superscript\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-position=\"super 58%%\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Subscript\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-position=\"sub 58%%\"/>\n" \
    "   </style:style>\n" \
    "<style:style style:name=\"MMD-Table\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.05in\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"MMD-Table-Center\" style:family=\"paragraph\" style:parent-style-name=\"MMD-Table\">\n" \
    "   <style:paragraph-properties fo:text-align=\"center\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"MMD-Table-Right\" style:family=\"paragraph\" style:parent-style-name=\"MMD-Table\">\n" \
    "   <style:paragraph-properties fo:text-align=\"right\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"P2\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"\n" \
    "             style:list-style-name=\"L2\">\n" \
    "<style:paragraph-properties fo:text-align=\"start\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
	"<style:style style:name=\"fr1\" style:family=\"graphic\" style:parent-style-name=\"Frame\">\n" \
	"   <style:graphic-properties style:print-content=\"false\" style:vertical-pos=\"top\"\n" \
	"                             style:vertical-rel=\"baseline\"\n" \
	"                             fo:padding=\"0in\"\n" \
	"                             fo:border=\"none\"\n" \
	"                             style:shadow=\"none\"/>\n" \
	"</style:style>\n" \
    "<style:style style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"\n" \
    "             style:list-style-name=\"L1\"/>\n" \
	"<text:list-style style:name=\"L1\">\n" \
	"	<text:list-level-style-bullet text:level=\"1\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"		<style:text-properties fo:font-family=\"starbats\" style:font-charset=\"x-symbol\"/>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-bullet text:level=\"2\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"		<style:text-properties fo:font-family=\"starbats\" style:font-charset=\"x-symbol\"/>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-bullet text:level=\"3\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"		<style:text-properties fo:font-family=\"starbats\" style:font-charset=\"x-symbol\"/>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-number text:level=\"4\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"5\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"6\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"7\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"8\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"9\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"10\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"</text:list-style>\n" \
	"<text:list-style style:name=\"L2\">\n" \
	"	<text:list-level-style-number text:level=\"1\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"2\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"3\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"4\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"5\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"6\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"7\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"8\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"9\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"10\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"</text:list-style>\n" \
    "</office:automatic-styles>\n" \
	" <office:master-styles>\n" \
	"  <style:master-page style:name=\"Endnote\" >\n" \
	"    <style:header><text:h text:outline-level=\"2\">Bibliography</text:h></style:header></style:master-page>\n" \
	"  <style:master-page style:name=\"Footnote\" style:page-layout-name=\"pm2\"/>\n" \
	" </office:master-styles>\n") ;
}

void print_odf_footer(GString *out) {
    g_string_append_printf(out, "</office:text>\n</office:body>\n</office:document>");
}
