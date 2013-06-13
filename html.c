/*

	html.c -- HTML writer

	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).

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

#include "html.h"

bool is_html_complete_doc(node *meta);


/* print_html_node_tree -- convert node tree to HTML */
void print_html_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_html_node(out, list, scratch);
		list = list->next;
	}
}

/* print_html_node -- convert given node to HTML and append */
void print_html_node(GString *out, node *n, scratch_pad *scratch) {
	node *temp_node;
	char *temp;
	int lev;
	char *width = NULL;
	char *height = NULL;
	GString *temp_str;

	if (n == NULL)
		return;
	
	/* debugging statement */
#ifdef DEBUG_ON
	fprintf(stderr, "print_html_node: %d\n",n->key);
#endif
	
	/* If we are forcing a complete document, and METADATA isn't the first thing,
		we need to close <head> */
	if ((scratch->extensions & EXT_COMPLETE)
		&& !(scratch->extensions & EXT_HEAD_CLOSED) && 
		!((n->key == FOOTER) || (n->key == METADATA))) {
			g_string_append_printf(out, "</head>\n<body>\n");
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
		}
	switch (n->key) {
		case NO_TYPE:
			break;
		case LIST:
			print_html_node_tree(out,n->children,scratch);
			break;
		case STR:
			print_html_string(out,n->str, scratch);
			break;
		case SPACE:
			g_string_append_printf(out,"%s",n->str);
			break;
		case PLAIN:
			pad(out,1, scratch);
			print_html_node_tree(out,n->children, scratch);
			scratch->padded = 0;
			break;
		case PARA:
			pad(out, 2, scratch);
			g_string_append_printf(out, "<p>");
			print_html_node_tree(out,n->children,scratch);
			if (scratch->footnote_to_print != 0) {
				g_string_append_printf(out, " <a href=\"#fnref:%d\" title=\"return to article\" class=\"reversefootnote\">&#160;&#8617;</a>", scratch->footnote_to_print);
				scratch->footnote_to_print = 0;
			}
			g_string_append_printf(out, "</p>");
			scratch->padded = 0;
			break;
		case HRULE:
			pad(out, 2, scratch);
			g_string_append_printf(out, "<hr />");
			scratch->padded = 0;
			break;
		case HTMLBLOCK:
			pad(out, 2, scratch);
			g_string_append_printf(out, "%s", n->str);
			scratch->padded = 0;
			break;
		case VERBATIM:
			pad(out, 2, scratch);
			g_string_append_printf(out, "%s", "<pre><code>");
			print_html_string(out, n->str, scratch);
			g_string_append_printf(out, "%s", "</code></pre>");
			scratch->padded = 0;
			break;
		case BULLETLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "%s", "<ul>");
			scratch->padded = 0;
			print_html_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "%s", "</ul>");
			scratch->padded = 0;
			break;
		case ORDEREDLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "%s", "<ol>");
			scratch->padded = 0;
			print_html_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "</ol>");
			scratch->padded = 0;
			break;
		case LISTITEM:
			pad(out, 1, scratch);
			g_string_append_printf(out, "<li>");
			scratch->padded = 2;
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</li>");
			scratch->padded = 0;
			break;
		case METADATA:
			if (!(scratch->extensions & EXT_COMPLETE) && (is_html_complete_doc(n))) {
				/* We have metadata to include, and didn't already force complete */
				g_string_append_printf(out,
				"<!DOCTYPE html>\n<html>\n<head>\n\t<meta charset=\"utf-8\"/>\n");
				/* either way, now we need to be a complete doc */
				scratch->extensions = scratch->extensions | EXT_COMPLETE;
			}
			/* print the metadata */
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
			print_html_node_tree(out,n->children, scratch);
			if (scratch->extensions & EXT_COMPLETE) {
				/* need to close head and open body */
				g_string_append_printf(out, "</head>\n<body>\n\n");
			}
			break;
		case METAKEY:
			if (strcmp(n->str, "title") == 0) {
				g_string_append_printf(out, "\t<title>");
				print_html_node(out, n->children, scratch);
				g_string_append_printf(out, "</title>\n");
			} else if (strcmp(n->str, "css") == 0) {
				g_string_append_printf(out, "\t<link type=\"text/css\" rel=\"stylesheet\" href=\"");
				print_html_node(out, n->children, scratch);
				g_string_append_printf(out, "\"/>\n");
			} else if (strcmp(n->str, "xhtmlheader") == 0) {
				trim_trailing_whitespace(n->children->str);
				print_raw_node(out, n->children);
				g_string_append_printf(out, "\n");
			} else if (strcmp(n->str, "htmlheader") == 0) {
				trim_trailing_whitespace(n->children->str);
				print_raw_node(out, n->children);
				g_string_append_printf(out, "\n");
			} else if (strcmp(n->str, "baseheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
			} else if (strcmp(n->str, "xhtmlheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
			} else if (strcmp(n->str, "htmlheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
			} else if (strcmp(n->str, "quoteslanguage") == 0) {
				temp = label_from_node_tree(n->children);
				if ((strcmp(temp, "nl") == 0) || (strcmp(temp, "dutch") == 0)) { scratch->language = DUTCH; }   else 
				if ((strcmp(temp, "de") == 0) || (strcmp(temp, "german") == 0)) { scratch->language = GERMAN; } else 
				if (strcmp(temp, "germanguillemets") == 0) { scratch->language = GERMANGUILL; } else 
				if ((strcmp(temp, "fr") == 0) || (strcmp(temp, "french") == 0)) { scratch->language = FRENCH; } else 
				if ((strcmp(temp, "sv") == 0) || (strcmp(temp, "swedish") == 0)) { scratch->language = SWEDISH; }
				free(temp);
			} else {
				g_string_append_printf(out,"\t<meta name=\"%s\" content=\"",n->str);
				print_html_node(out,n->children,scratch);
				g_string_append_printf(out,"\"/>\n");
			}
			break;
		case METAVALUE:
			trim_trailing_whitespace(n->str);
			print_html_string(out,n->str, scratch);
			break;
		case FOOTER:
			break;
		case HEADINGSECTION:
			print_html_node_tree(out,n->children,scratch);
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;  /* assumes H1 ... H6 are in order */
			if (lev > 6)
				lev = 6;	/* Max at level 6 */
			pad(out, 2, scratch);
			if ( scratch->extensions & EXT_COMPATIBILITY ) {
				/* Use regular Markdown header format */
				g_string_append_printf(out, "<h%1d>", lev);
				print_html_node_tree(out, n->children, scratch);
			} else if (n->children->key == AUTOLABEL) {
				/* use label for header since one was specified (MMD)*/
				g_string_append_printf(out, "<h%d id=\"%s\">", lev,n->children->str);
				print_html_node_tree(out, n->children->next, scratch);
			} else if ( scratch->extensions & EXT_NO_LABELS ) {
				/* Don't generate a label */
				g_string_append_printf(out, "<h%1d>", lev);
				print_html_node_tree(out, n->children, scratch);
			} else {
				/* generate a label by default for MMD */
				temp = label_from_node_tree(n->children);
				g_string_append_printf(out, "<h%d id=\"%s\">", lev, temp);
				print_html_node_tree(out, n->children, scratch);
				free(temp);
			}
			g_string_append_printf(out, "</h%1d>", lev);
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
			print_html_node_tree(out, n->children, scratch);
			print_html_localized_typography(out, RSQUOTE, scratch);
			break;
		case DOUBLEQUOTED:
			print_html_localized_typography(out, LDQUOTE, scratch);
			print_html_node_tree(out, n->children, scratch);
			print_html_localized_typography(out, RDQUOTE, scratch);
			break;
		case LINEBREAK:
			g_string_append_printf(out, "<br/>\n");
			break;
		case MATHSPAN:
			if (n->str[0] == '$') {
				n->str[strlen(n->str)-1] = '\0';
				if (n->str[1] == '$') {
					n->str[strlen(n->str)-1] = '\0';
					g_string_append_printf(out, "<span class=\"math\">\\[%s\\]</span>",&n->str[2]);
				} else {
					g_string_append_printf(out, "<span class=\"math\">\\(%s\\)</span>",&n->str[1]);
				}
			} else if (n->str[strlen(n->str) - 1] == ']') {
				n->str[strlen(n->str) - 3] = '\0';
				g_string_append_printf(out, "<span class=\"math\">%s\\]</span>",n->str);
			} else {
				n->str[strlen(n->str) - 3] = '\0';
				g_string_append_printf(out, "<span class=\"math\">%s\\)</span>",n->str);
			}
			break;
		case STRONG:
			g_string_append_printf(out, "<strong>");
			print_html_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "</strong>");
		break;
		case EMPH:
			g_string_append_printf(out, "<em>");
			print_html_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "</em>");
		break;
		case LINKREFERENCE:
			break;
		case LINK:
#ifdef DEBUG_ON
	fprintf(stderr, "print html link: '%s'\n",n->str);
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
					g_string_append_printf(out, "[");
					print_html_node(out, n->children, scratch);
					g_string_append_printf(out,"]");
					if (n->children->next != NULL) {
						g_string_append_printf(out, "[");
						print_html_node_tree(out, n->children->next, scratch);
						g_string_append_printf(out,"]");
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
			g_string_append_printf(out, "<a");
			if (n->link_data->source != NULL) {
				g_string_append_printf(out, " href=\"");
				if (strncmp(n->link_data->source,"mailto:", 6) == 0) {
					scratch->obfuscate = 1;		/* flag obfuscated */
				}
				print_html_string(out,n->link_data->source, scratch);
				g_string_append_printf(out, "\"");
			}
			if ((n->link_data->title != NULL) && (strlen(n->link_data->title) > 0)) {
				g_string_append_printf(out, " title=\"");
				print_html_string(out, n->link_data->title, scratch);
				g_string_append_printf(out, "\"");
			}
			print_html_node_tree(out, n->link_data->attr, scratch);
			g_string_append_printf(out, ">");
			if (n->children != NULL)
				print_html_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "</a>");
			n->link_data->attr = NULL;	/* We'll delete these elsewhere */
			scratch->obfuscate = 0;
			break;
		case ATTRKEY:
			if ( (strcmp(n->str,"height") == 0) || (strcmp(n->str, "width") == 0)) {
			} else {
				g_string_append_printf(out, " %s=\"%s\"", n->str,
					n->children->str);
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
			if (n->key == IMAGEBLOCK)
				g_string_append_printf(out, "<figure>\n");
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
					free(temp);
					break;
				}
				free(temp);
			}
#ifdef DEBUG_ON
	fprintf(stderr, "create img\n");
#endif
			g_string_append_printf(out, "<img");
			if (n->link_data->source != NULL)
				g_string_append_printf(out, " src=\"%s\"",n->link_data->source);
			if (n->children != NULL) {
				g_string_append_printf(out, " alt=\"");
				temp_str = g_string_new("");
				print_raw_node_tree(temp_str, n->children);
				print_html_string(out, temp_str->str, scratch);
				g_string_free(temp_str, true);
				g_string_append_printf(out, "\"");
			} else {
				g_string_append_printf(out, " alt=\"%s\"",n->link_data->title);
			}
			if (!(scratch->extensions & EXT_COMPATIBILITY)) {
				if (n->link_data->label != NULL)
					g_string_append_printf(out, " id=\"%s\"",n->link_data->label);
			}
			if ((n->link_data->title != NULL) && (strlen(n->link_data->title) > 0)) {
				g_string_append_printf(out, " title=\"");
				print_html_string(out, n->link_data->title, scratch);
				g_string_append_printf(out, "\"");
			}
#ifdef DEBUG_ON
	fprintf(stderr, "attributes\n");
#endif
			if (n->link_data->attr != NULL) {
				temp_node = node_for_attribute("height",n->link_data->attr);
				if (temp_node != NULL)
					height = strdup(temp_node->children->str);
				temp_node = node_for_attribute("width",n->link_data->attr);
				if (temp_node != NULL)
					width = strdup(temp_node->children->str);
				if ((height != NULL) || (width != NULL)) {
	#ifdef DEBUG_ON
		fprintf(stderr, "width/height\n");
	#endif
				g_string_append_printf(out, " style=\"");
				if (height != NULL)
					g_string_append_printf(out, "height:%s;", height);
				if (width != NULL)
					g_string_append_printf(out, "width:%s;", width);
				g_string_append_printf(out, "\"");
			}
	#ifdef DEBUG_ON
		fprintf(stderr, "other attributes\n");
	#endif
				print_html_node_tree(out, n->link_data->attr, scratch);
				free(height);
				free(width);
			}
			g_string_append_printf(out, " />");
			if (n->key == IMAGEBLOCK) {
				if (n->children != NULL) {
					g_string_append_printf(out, "\n<figcaption>");
					print_html_node(out,n->children,scratch);
					g_string_append_printf(out, "</figcaption>");
				}
				g_string_append_printf(out,"</figure>");
				scratch->padded = 0;
			}
			n->link_data->attr = NULL;	/* We'll delete these elsewhere */
			break;
#ifdef DEBUG_ON
	fprintf(stderr, "finish image\n");
#endif
		case NOTEREFERENCE:
			lev = note_number_for_label(n->str, scratch);
			temp_node = node_for_count(scratch->used_notes, lev);
			if (temp_node->key == GLOSSARYSOURCE) {
				if (lev > scratch->max_footnote_num) {
					g_string_append_printf(out, "<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"see footnote\" class=\"footnote glossary\">[%d]</a>",
						lev, lev, lev);
						scratch->max_footnote_num = lev;
				} else {
					g_string_append_printf(out, "<a href=\"#fn:%d\" title=\"see footnote\" class=\"footnote glossary\">[%d]</a>",
						lev, lev, lev);				
				}
			} else {
				if (lev > scratch->max_footnote_num) {
					g_string_append_printf(out, "<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"see footnote\" class=\"footnote\">[%d]</a>",
						lev, lev, lev);
						scratch->max_footnote_num = lev;
				} else {
					g_string_append_printf(out, "<a href=\"#fn:%d\" title=\"see footnote\" class=\"footnote\">[%d]</a>",
						lev, lev, lev);
				}
			}
			break;
		case NOCITATION:
		case CITATION:
#ifdef DEBUG_ON
	fprintf(stderr, "print cite\n");
#endif
			if ((n->link_data != NULL) && (strncmp(n->link_data->label,"[#",2) == 0)) {
				fprintf(stderr, "external cite\n");
				/* external citation (e.g. BibTeX) */
				if (n->key == NOCITATION) {
					g_string_append_printf(out, "<span class=\"notcited\" id=\"%s\"/>",n->str);
				} else {
					g_string_append_printf(out, "<span class=\"externalcitation\">");
					g_string_append_printf(out, "</span>");
				}
			} else {
#ifdef DEBUG_ON
				fprintf(stderr, "internal cite\n");
#endif
				/* MMD citation, so output as footnote */
				/* TODO: create separate stream from footnotes */
				lev = 0;
				if (n->link_data != NULL)
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
						g_string_append_printf(out, "<span class=\"notcited\" id=\"%d\">",lev);
					} else {
						g_string_append_printf(out, "<a class=\"citation\" href=\"#fn:%d\" title=\"Jump to citation\">[",
							lev);
							if (n->children != NULL) {
								g_string_append_printf(out, "<span class=\"locator\">");
								print_html_node(out, n->children, scratch);
								g_string_append_printf(out, "</span>, %d]", lev);
							} else {
								g_string_append_printf(out, "%d]", lev);
							}
					}
					g_string_append_printf(out, "<span class=\"citekey\" style=\"display:none\">%s</span>", n->link_data->label);
					if (n->key == NOCITATION) {
						g_string_append_printf(out, "</span>");
					} else {
						g_string_append_printf(out, "</a>");
					}
				} else {
					/* not located -- this is external cite */
#ifdef DEBUG_ON
				fprintf(stderr, "no match for cite: '%s'\n",n->link_data->label);
#endif
					if ((n->link_data != NULL) && (n->key == NOCITATION)) {
						g_string_append_printf(out, "<span class=\"notcited\" id=\"%s\"/>",n->link_data->label);
					} else if (n->link_data != NULL) {
						g_string_append_printf(out, "<span class=\"externalcitation\">[");
						if (n->children != NULL) {
							print_html_node(out, n->children, scratch);
							g_string_append_printf(out, "][");
						}
						g_string_append_printf(out, "#%s]</span>",n->link_data->label);
					}
				}
			}
#ifdef DEBUG_ON
		fprintf(stderr, "finish cite\n");
#endif
			break;
		case GLOSSARYTERM:
			g_string_append_printf(out,"<span class=\"glossary name\">");
			print_html_string(out, n->children->str, scratch);
			g_string_append_printf(out, "</span>");
			if ((n->next != NULL) && (n->next->key == GLOSSARYSORTKEY) ) {
				g_string_append_printf(out, "<span class=\"glossary sort\" style=\"display:none\">");
				print_html_string(out, n->next->str, scratch);
				g_string_append_printf(out, "</span>");
			}
			g_string_append_printf(out, ": ");
			break;
		case GLOSSARYSORTKEY:
			break;
		case CODE:
			g_string_append_printf(out, "<code>");
			print_html_string(out, n->str, scratch);
			g_string_append_printf(out, "</code>");
			break;
		case BLOCKQUOTEMARKER:
			print_html_node_tree(out, n->children, scratch);
			break;
		case BLOCKQUOTE:
			pad(out,2, scratch);
			g_string_append_printf(out, "<blockquote>\n");
			scratch->padded = 2;
			print_html_node_tree(out, n->children, scratch);
			pad(out,1, scratch);
			g_string_append_printf(out, "</blockquote>");
			scratch->padded = 0;
			break;
		case RAW:
			g_string_append_printf(out, "RAW:");
			g_string_append_printf(out,"%s",n->str);
			break;
		case HTML:
			g_string_append_printf(out, "%s", n->str);
			break;
		case DEFLIST:
			pad(out,2, scratch);
			scratch->padded = 1;
			g_string_append_printf(out, "<dl>\n");
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</dl>");
			scratch->padded = 0;
			break;
		case TERM:
			pad(out,1, scratch);
			g_string_append_printf(out, "<dt>");
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</dt>\n");
			scratch->padded = 1;
			break;
		case DEFINITION:
			pad(out,1, scratch);
			scratch->padded = 1;
			g_string_append_printf(out, "<dd>");
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</dd>\n");
			scratch->padded = 0;
			break;
		case TABLE:
			pad(out,2, scratch);
			g_string_append_printf(out, "<table>\n");
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</table>\n");
			scratch->padded = 1;
			break;
		case TABLESEPARATOR:
			scratch->table_alignment = n->str;
			break;
		case TABLECAPTION:
			if ((n->children != NULL) && (n->children->key == TABLELABEL)) {
				temp = label_from_string(n->children->str);
			} else {
				temp = label_from_node_tree(n->children);
			}
			g_string_append_printf(out, "<caption id=\"%s\">", temp);
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</caption>\n");
			free(temp);
			break;
		case TABLELABEL:
			break;
		case TABLEHEAD:
			/* print column alignment for XSLT processing if needed */
			g_string_append_printf(out, "<colgroup>\n");
			temp = scratch->table_alignment;
			for (lev=0;lev<strlen(temp);lev++) {
				if ( strncmp(&temp[lev],"r",1) == 0) {
					g_string_append_printf(out, "<col style=\"text-align:right;\"/>\n");
				} else if ( strncmp(&temp[lev],"R",1) == 0) {
					g_string_append_printf(out, "<col style=\"text-align:right;\" class=\"extended\"/>\n");
				} else if ( strncmp(&temp[lev],"c",1) == 0) {
					g_string_append_printf(out, "<col style=\"text-align:center;\"/>\n");
				} else if ( strncmp(&temp[lev],"C",1) == 0) {
					g_string_append_printf(out, "<col style=\"text-align:center;\" class=\"extended\"/>\n");
				} else if ( strncmp(&temp[lev],"L",1) == 0) {
					g_string_append_printf(out, "<col style=\"text-align:left;\" class=\"extended\"/>\n");
				} else {
					g_string_append_printf(out, "<col style=\"text-align:left;\"/>\n");
				}
			}
			g_string_append_printf(out, "</colgroup>\n");
			scratch->cell_type = 'h';
			g_string_append_printf(out, "\n<thead>\n");
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</thead>\n");
			scratch->cell_type = 'd';
			break;
		case TABLEBODY:
			g_string_append_printf(out, "\n<tbody>\n");
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</tbody>\n");
			break;
		case TABLEROW:
			g_string_append_printf(out, "<tr>\n");
			scratch->table_column = 0;
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</tr>\n");
			break;
		case TABLECELL:
			temp = scratch->table_alignment;
			lev = scratch->table_column;
			if ( strncmp(&temp[lev],"r",1) == 0) {
				g_string_append_printf(out, "\t<t%c style=\"text-align:right;\"", scratch->cell_type);
			} else if ( strncmp(&temp[lev],"R",1) == 0) {
				g_string_append_printf(out, "\t<t%c style=\"text-align:right;\"", scratch->cell_type);
			} else if ( strncmp(&temp[lev],"c",1) == 0) {
				g_string_append_printf(out, "\t<t%c style=\"text-align:center;\"", scratch->cell_type);
			} else if ( strncmp(&temp[lev],"C",1) == 0) {
				g_string_append_printf(out, "\t<t%c style=\"text-align:center;\"", scratch->cell_type);
			} else {
				g_string_append_printf(out, "\t<t%c style=\"text-align:left;\"", scratch->cell_type);
			}
			if ((n->children != NULL) && (n->children->key == CELLSPAN)) {
				g_string_append_printf(out, " colspan=\"%d\"",(int)strlen(n->children->str)+1);
			}
			g_string_append_printf(out, ">");
			scratch->padded = 2;
			print_html_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "</t%c>\n", scratch->cell_type);
			scratch->table_column++;
			break;
		case CELLSPAN:
			break;
		case GLOSSARYSOURCE:
			print_html_node_tree(out, n->children, scratch);
			break;
		case CITATIONSOURCE:
		case NOTESOURCE:
			print_html_node(out, n->children, scratch);
			break;
		case SOURCEBRANCH:
			fprintf(stderr,"SOURCEBRANCH\n");
			break;
		case NOTELABEL:
			break;
		case KEY_COUNTER:
			break;
		default:
			fprintf(stderr, "print_html_node encountered unknown node key = %d\n",n->key);
			exit(EXIT_FAILURE);
	}
}

/* print_html_endnotes */
void print_html_endnotes(GString *out, scratch_pad *scratch) {
	int counter = 0;
	scratch->used_notes = reverse_list(scratch->used_notes);
	node *note = scratch->used_notes;
#ifdef DEBUG_ON
	fprintf(stderr, "start endnotes\n");
#endif
	
	if ((note == NULL) || ((note->key == KEY_COUNTER) && (note->next == NULL)))
		return;

#ifdef DEBUG_ON
	fprintf(stderr, "there are endnotes to print\n");
#endif

	pad(out,2, scratch);
	g_string_append_printf(out, "<div class=\"footnotes\">\n<hr />\n<ol>");
	while ( note != NULL) {
		if (note->key == KEY_COUNTER) {
			note = note->next;
			continue;
		}
		
		counter++;
		pad(out, 1, scratch);
		
		if (note->key == CITATIONSOURCE) {
			g_string_append_printf(out, "<li id=\"fn:%d\" class=\"citation\"><span class=\"citekey\" style=\"display:none\">%s</span>", 
				counter, note->str);
		} else {
			g_string_append_printf(out, "<li id=\"fn:%d\">\n", counter);
		}
		scratch->padded = 2;
		if ((note->key == NOTESOURCE) || (note->key == GLOSSARYSOURCE))
			scratch->footnote_to_print = counter;
		print_html_node(out, note, scratch);
		pad(out, 1, scratch);
		g_string_append_printf(out, "</li>");
		
		note = note->next;
	}
	pad(out,1, scratch);
	g_string_append_printf(out, "</ol>\n</div>\n");
	scratch->padded = 0;
#ifdef DEBUG_ON
	fprintf(stderr, "finish endnotes\n");
#endif
}

/* Check metadata keys and determine if we need a complete document */
/* We also preconvert metadata keys to proper formatting -- lowercase with no spaces */
bool is_html_complete_doc(node *meta) {
	node *step;
	char *temp;
	step = meta->children;

	while (step != NULL) {
		/* process key to proper label */
		temp = step->str;	/* store pointer to original str */
		step->str = label_from_string(step->str);
		free(temp);			/* free original since we don't need it */
		step = step->next;
	}

	step = meta->children;
	while (step != NULL) {
		/* the following types of metadata do not require a complete document */
		if ((strcmp(step->str, "baseheaderlevel")  != 0) &&
			(strcmp(step->str, "xhtmlheaderlevel") != 0) &&
			(strcmp(step->str, "htmlheaderlevel")  != 0) &&
			(strcmp(step->str, "latexheaderlevel") != 0) &&
			(strcmp(step->str, "odfheaderlevel")   != 0) &&
			(strcmp(step->str, "quoteslanguage")   != 0))
			{ return TRUE;}
		step = step->next;
	}

	return FALSE;
}

/* print_html_localized_typography -- convert to "smart" typography */
void print_html_localized_typography(GString *out, int character, scratch_pad *scratch) {
	if (!extension(EXT_SMART, scratch->extensions)) {
		g_string_append_c(out, character);
		return;
	}
	switch (character) {
		case LSQUOTE:
			switch (scratch->language) {
				case SWEDISH:
					g_string_append_printf(out, "&#8217;");
					break;
				case FRENCH:
					g_string_append_printf(out,"&#39;");
					break;
				case GERMAN:
					g_string_append_printf(out,"&#8218;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#8250;");
					break;
				default:
					g_string_append_printf(out,"&#8216;");
				}
			break;
		case RSQUOTE:
			switch (scratch->language) {
				case GERMAN:
					g_string_append_printf(out,"&#8216;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#8249;");
					break;
				default:
					g_string_append_printf(out,"&#8217;");
				}
			break;
		case APOS:
			g_string_append_printf(out,"&#8217;");
			break;
		case LDQUOTE:
			switch (scratch->language) {
				case DUTCH:
				case GERMAN:
					g_string_append_printf(out,"&#8222;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#187;");
					break;
				case FRENCH:
					g_string_append_printf(out,"&#171;");
					break;
				case SWEDISH:
					g_string_append_printf(out, "&#8221;");
					break;
				default:
					g_string_append_printf(out,"&#8220;");
				}
			break;
		case RDQUOTE:
			switch (scratch->language) {
				case SWEDISH:
				case DUTCH:
					g_string_append_printf(out,"&#8221;");
					break;
				case GERMAN:
					g_string_append_printf(out,"&#8220;");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"&#171;");
					break;
				case FRENCH:
					g_string_append_printf(out,"&#187;");
					break;
				default:
					g_string_append_printf(out,"&#8221;");
				}
			break;
		case NDASH:
			g_string_append_printf(out,"&#8211;");
			break;
		case MDASH:
			g_string_append_printf(out,"&#8212;");
			break;
		case ELLIP:
			g_string_append_printf(out,"&#8230;");
			break;
			default:;
	}
}

/* print_html_string - print string, escaping for HTML */
void print_html_string(GString *out, char *str, scratch_pad *scratch) {
	if (str == NULL)
		return;
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
			default:
				if ((scratch->obfuscate == true) && (extension(EXT_OBFUSCATE, scratch->extensions))
					&& ((int) *str == (((int) *str) & 127))) { 
					if (rand() % 2 == 0)
						g_string_append_printf(out, "&#%d;", (int) *str);
					else
						g_string_append_printf(out, "&#x%x;", (unsigned int) *str);
				} else {
					g_string_append_c(out, *str);
				}
		}
		str++;
	}
}
