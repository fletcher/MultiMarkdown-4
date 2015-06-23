/*

	latex.c -- LaTeX writer

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

#include "latex.h"

bool is_latex_complete_doc(node *meta);

/* find_latex_mode -- check for metadata to switch to beamer/memoir */
int find_latex_mode(int format, node *n) {
	node *latex_mode;
	char *key;
	char *label;
	
	if (format != LATEX_FORMAT)
		return format;
	
	if (tree_contains_key(n, METAKEY)) {
		latex_mode = metadata_for_key("latexmode",n);
		if (latex_mode != NULL) {
			key = metavalue_for_key("latexmode",n);
			label = label_from_string(key);
			if (strcmp(label, "beamer") == 0) {
				format = BEAMER_FORMAT;
			} else if (strcmp(label, "memoir") == 0) {
				format = MEMOIR_FORMAT;
			}
			free(label);
			free(key);
		}
	}
	
	return format;
}


/* print_latex_node_tree -- convert node tree to LaTeX */
void print_latex_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_latex_node(out, list, scratch);
		list = list->next;
	}
}

/* print_latex_node -- convert given node to LaTeX and append */
void print_latex_node(GString *out, node *n, scratch_pad *scratch) {
	node *temp_node;
	char *temp;
	int lev;
	char *width = NULL;
	char *height = NULL;
	GString *temp_str;
	GString *raw_str;
	int i;
	double temp_float;

	if (n == NULL)
		return;
	
	/* debugging statement */
#ifdef DEBUG_ON
	fprintf(stderr, "print_latex_node: %d\n",n->key);
#endif
	
	/* If we are forcing a complete document, and METADATA isn't the first thing,
		we need to close <head> */
	if ((scratch->extensions & EXT_COMPLETE)
		&& !(scratch->extensions & EXT_HEAD_CLOSED) && 
		!((n->key == FOOTER) || (n->key == METADATA))) {
			pad(out, 2, scratch);
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
		}
	switch (n->key) {
		case NO_TYPE:
			break;
		case LIST:
			print_latex_node_tree(out,n->children,scratch);
			break;
		case STR:
			print_latex_string(out,n->str, scratch);
			break;
		case ABBREVIATION:
			/* We combine the short and full names, since stripping non-ascii characters may result
				in a conflict otherwise.  This at least makes it less likely. */
			width = ascii_label_from_node(n->children);
			temp = ascii_label_from_string(n->str);
			g_string_append_printf(out, "\\newacro{%s%s}[",width,temp);
			print_latex_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "]{");
			trim_trailing_whitespace(n->str);
			print_latex_string(out, n->str, scratch);
			g_string_append_printf(out, "}\n");
			free(temp);
			free(width);
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
			width = ascii_label_from_node(n->children->children);
			temp = ascii_label_from_string(n->children->str);
			g_string_append_printf(out, "\\ac{%s%s}", width, temp);
			free(temp);
			free(width);
			break;
		case ABBRSTOP:
			break;		
		case SPACE:
			g_string_append_printf(out,"%s",n->str);
			break;
		case PLAIN:
			pad(out,1, scratch);
			print_latex_node_tree(out,n->children, scratch);
			scratch->padded = 0;
			break;
		case PARA:
			pad(out, 2, scratch);
			print_latex_node_tree(out,n->children,scratch);
			scratch->padded = 0;
			break;
		case HRULE:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{center}\\rule{3in}{0.4pt}\\end{center}\n");
			scratch->padded = 0;
			break;
		case HTMLBLOCK:
			/* don't print HTML block */
			/* but do print HTML comments for raw LaTeX */
			if (strncmp(n->str,"<!--",4) == 0) {
				pad(out, 2, scratch);
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "%s", &n->str[4]);
				scratch->padded = 0;
			}
			break;
		case VERBATIM:
		case VERBATIMFENCE:
			pad(out, 2, scratch);
			if ((n->children != NULL) && (n->children->key == VERBATIMTYPE)) {
				trim_trailing_whitespace(n->children->str);
				if (strlen(n->children->str) > 0) {
					g_string_append_printf(out, "\\begin{lstlisting}[language=%s]\n%s\\end{lstlisting}", n->children->str,n->str);
					scratch->padded = 0;
					break;
				}
			}
			g_string_append_printf(out, "\\begin{verbatim}\n%s\\end{verbatim}",n->str);
			scratch->padded = 0;
			break;
		case BULLETLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{itemize}");
			scratch->padded = 0;
			print_latex_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\end{itemize}");
			scratch->padded = 0;
			break;
		case ORDEREDLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{enumerate}");
			scratch->padded = 0;
			print_latex_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\end{enumerate}");
			scratch->padded = 0;
			break;
		case LISTITEM:
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\item ");
			scratch->padded = 2;
			print_latex_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\n");
			scratch->padded = 0;
			break;
		case METADATA:
			/* print the metadata */
			print_latex_node_tree(out,n->children, scratch);
			if (!(scratch->extensions & EXT_SNIPPET) && (is_latex_complete_doc(n))) {
				scratch->extensions = scratch->extensions | EXT_COMPLETE;
			}
			/* print acronym definitions */
			print_latex_node_tree(out, scratch->abbreviations, scratch);
			break;
		case METAKEY:
			/* reformat the key */
			temp = n->str;
			n->str = label_from_string(temp);
			free(temp);
			
			if (strcmp(n->str, "baseheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
				break;
			} else if (strcmp(n->str, "latexheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
				break;
			} else if (strcmp(n->str, "quoteslanguage") == 0) {
				temp = label_from_node_tree(n->children);
				if ((strcmp(temp, "nl") == 0) || (strcmp(temp, "dutch") == 0)) { scratch->language = DUTCH; }   else 
				if ((strcmp(temp, "de") == 0) || (strcmp(temp, "german") == 0)) { scratch->language = GERMAN; } else 
				if (strcmp(temp, "germanguillemets") == 0) { scratch->language = GERMANGUILL; } else 
				if ((strcmp(temp, "fr") == 0) || (strcmp(temp, "french") == 0)) { scratch->language = FRENCH; } else 
				if ((strcmp(temp, "sv") == 0) || (strcmp(temp, "swedish") == 0)) { scratch->language = SWEDISH; }
				break;
			}
			
			/* Don't handle remaining metadata if we're snippet only */
			
			if (scratch->extensions & EXT_SNIPPET)
				break;
							
			if (strcmp(n->str, "title") == 0) {
				g_string_append_printf(out, "\\def\\mytitle{");
				print_latex_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "latextitle") == 0) {
				g_string_append_printf(out, "\\def\\mytitle{%s}\n",n->children->str);
			} else if (strcmp(n->str, "author") == 0) {
				g_string_append_printf(out, "\\def\\myauthor{");
				print_latex_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "latexauthor") == 0) {
				g_string_append_printf(out, "\\def\\myauthor{%s}\n",n->children->str);
			} else if (strcmp(n->str, "date") == 0) {
				g_string_append_printf(out, "\\def\\mydate{");
				print_latex_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "copyright") == 0) {
				g_string_append_printf(out, "\\def\\mycopyright{");
				print_latex_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "css") == 0) {
			} else if (strcmp(n->str, "xhtmlheader") == 0) {
			} else if (strcmp(n->str, "htmlheader") == 0) {
			} else if (strcmp(n->str, "mmdfooter") == 0) {
			} else if (strcmp(n->str, "mmdheader") == 0) {
			} else if (strcmp(n->str, "lang") == 0) {
			} else if (strcmp(n->str, "latexinput") == 0) {
				trim_trailing_whitespace(n->children->str);
				g_string_append_printf(out, "\\input{%s}\n", n->children->str);
			} else if (strcmp(n->str, "latexfooter") == 0) {
				trim_trailing_whitespace(n->children->str);
				scratch->latex_footer = strdup(n->children->str);
			} else if (strcmp(n->str, "bibtex") == 0) {
				trim_trailing_whitespace(n->children->str);
				g_string_append_printf(out, "\\def\\bibliocommand{\\bibliography{%s}}\n",n->children->str);
			} else {
				g_string_append_printf(out, "\\def\\");
				print_latex_string(out, n->str, scratch);
				g_string_append_printf(out, "{");
				print_latex_node_tree(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			}
			break;
		case METAVALUE:
			trim_trailing_whitespace(n->str);
			print_latex_string(out,n->str, scratch);
			break;
		case FOOTER:
			print_latex_endnotes(out, scratch);
			if (scratch->latex_footer != NULL) {
				pad(out, 2, scratch);
				g_string_append_printf(out,"\\input{%s}\n", scratch->latex_footer);
			}
			if (scratch->extensions & EXT_COMPLETE) {
				g_string_append_printf(out, "\n\\end{document}");
			}
			break;
		case HEADINGSECTION:
			print_latex_node_tree(out,n->children,scratch);
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;  /* assumes H1 ... H6 are in order */
			if (lev > 7)
				lev = 7;	/* Max at level 7 */
			pad(out, 2, scratch);
			switch (lev) {
				case 1:
					g_string_append_printf(out, "\\part{");
					break;
				case 2:
					g_string_append_printf(out, "\\chapter{");
					break;
				case 3:
					g_string_append_printf(out, "\\section{");
					break;
				case 4:
					g_string_append_printf(out, "\\subsection{");
					break;
				case 5:
					g_string_append_printf(out, "\\subsubsection{");
					break;
				case 6:
					g_string_append_printf(out, "\\paragraph{");
					break;
				case 7:
					g_string_append_printf(out, "\\subparagraph{");
					break;
			}
			/* Don't allow footnotes */
			scratch->no_latex_footnote = TRUE;
			if (n->children->key == AUTOLABEL) {
				/* use label for header since one was specified (MMD)*/
				temp = label_from_string(n->children->str);
				print_latex_node_tree(out, n->children->next, scratch);
				g_string_append_printf(out, "}\n\\label{%s}",temp);
				free(temp);
			} else {
				/* generate a label by default for MMD */
				temp = label_from_node_tree(n->children);
				print_latex_node_tree(out, n->children, scratch);
				g_string_append_printf(out, "}\n\\label{%s}",temp);
				free(temp);
			}
			scratch->no_latex_footnote = FALSE;
			scratch->padded = 0;
			break;
		case APOSTROPHE:
			print_latex_localized_typography(out, APOS, scratch);
			break;
		case ELLIPSIS:
			print_latex_localized_typography(out, ELLIP, scratch);
			break;
		case EMDASH:
			print_latex_localized_typography(out, MDASH, scratch);
			break;
		case ENDASH:
			print_latex_localized_typography(out, NDASH, scratch);
			break;
		case SINGLEQUOTED:
			print_latex_localized_typography(out, LSQUOTE, scratch);
			print_latex_node_tree(out, n->children, scratch);
			print_latex_localized_typography(out, RSQUOTE, scratch);
			break;
		case DOUBLEQUOTED:
			print_latex_localized_typography(out, LDQUOTE, scratch);
			print_latex_node_tree(out, n->children, scratch);
			print_latex_localized_typography(out, RDQUOTE, scratch);
			break;
		case LINEBREAK:
			g_string_append_printf(out, "\\\\\n");
			break;
		case MATHSPAN:
			temp = strdup(n->str);
			if (temp[0] == '$') {
				if (temp[1] == '$') {
					if (strncmp(&temp[2],"\\begin",5) == 0) {
						temp[strlen(temp)-2] = '\0';
						g_string_append_printf(out, "%s",&temp[1]);
					} else {
						g_string_append_printf(out, "%s",temp);
					}
				} else {
					if (strncmp(&temp[1],"\\begin",5) == 0) {
						temp[strlen(temp)-1] = '\0';
						g_string_append_printf(out, "%s",&temp[1]);
					} else {
						g_string_append_printf(out, "%s",temp);
					}
				}
			} else if (strncmp(&temp[2],"\\begin",5) == 0) {
				/* trim */
				temp[strlen(temp)-3] = '\0';
				g_string_append_printf(out, "%s", &temp[2]);
			} else {
				if (temp[strlen(temp)-1] == ']') {
					temp[strlen(temp)-3] = '\0';
					g_string_append_printf(out, "%s\\]", temp);
				} else {
					temp[strlen(temp)-3] = '\0';
					g_string_append_printf(out, "$%s$", &temp[2]);
				}
			}
			free(temp);
			break;
		case STRONG:
			g_string_append_printf(out, "\\textbf{");
			print_latex_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "}");
			break;
		case EMPH:
			g_string_append_printf(out, "\\emph{");
			print_latex_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "}");
			break;
		case LINKREFERENCE:
			break;
		case LINK:
#ifdef DEBUG_ON
	fprintf(stderr, "print LaTeX link: '%s'\n",n->str);
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
	fprintf(stderr, "print latex link: '%s'\n",n->str);
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
	fprintf(stderr, "look for reference data for latex link: '%s'\n",n->str);
#endif
			/* Load reference data */
			if (n->link_data->label != NULL) {
#ifdef DEBUG_ON
	fprintf(stderr, "have label for latex link: '%s'\n",n->str);
#endif
				temp = strdup(n->link_data->label);
				free_link_data(n->link_data);
				n->link_data = extract_link_data(temp, scratch);
				if (n->link_data == NULL) {
					/* replace original text since no definition found */
					g_string_append_printf(out, "[");
					print_latex_node(out, n->children, scratch);
					g_string_append_printf(out,"]");
					if (n->children->next != NULL) {
						g_string_append_printf(out, "[");
						print_latex_node_tree(out, n->children->next, scratch);
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
			temp_str = g_string_new("");
			print_latex_node_tree(temp_str, n->children, scratch);
			raw_str = g_string_new("");
			print_raw_node_tree(raw_str, n->children);
			
			if ((n->link_data->source != NULL) && (n->link_data->source[0] == '#' )) {
				/* link to anchor within the document */
				if (strlen(temp_str->str) > 0) {
					/* We have text before the link */
					g_string_append_printf(out, "%s (", temp_str->str);
				}
				
				if (n->link_data->label == NULL) {
					if ((n->link_data->source !=  NULL) && (strncmp(n->link_data->source,"#",1) == 0)) {
						/* This link was specified as [](#bar) */
						g_string_append_printf(out, "\\autoref{%s}", n->link_data->source + 1);
					} else {
						g_string_append_printf(out, "\\href{%s}{}", n->link_data->source);
					}
				} else {
					g_string_append_printf(out, "\\autoref{%s}", n->link_data->label);
				}
				if (strlen(temp_str->str) > 0) {
					g_string_append_printf(out, ")", temp_str->str);
				}
			} else if (strcmp(raw_str->str, n->link_data->source) == 0){
				/* This is a <link> */
				g_string_append_printf(out, "\\href{%s}{%s}", n->link_data->source, temp_str->str);
			} else if ((strlen(n->link_data->source) > 7) &&
				(strcmp(raw_str->str,&n->link_data->source[7]) == 0)) {
				/*This is a <mailto> */
				g_string_append_printf(out, "\\href{%s}{%s}", n->link_data->source, temp_str->str);
			} else {
				/* this is a [text](link) */
				g_string_append_printf(out, "\\href{%s}{", n->link_data->source);
				print_latex_node_tree(out, n->children, scratch);
				g_string_append_printf(out, "}");
				if (scratch->no_latex_footnote == FALSE) {
					g_string_append_printf(out, "\\footnote{\\href{");
					print_latex_url(out, n->link_data->source, scratch);
					g_string_append_printf(out, "}{", n->link_data->source);
					print_latex_string(out, n->link_data->source, scratch);
					g_string_append_printf(out, "}}");
				}
			}
			g_string_free(temp_str, true);
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
			pad(out,2, scratch);
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
					g_string_append_printf(out, "![");
					print_latex_node(out, n->children, scratch);
					g_string_append_printf(out,"]");
					if (n->children->next != NULL) {
						g_string_append_printf(out, "[");
						print_latex_node_tree(out, n->children->next, scratch);
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
			
			if (n->key == IMAGEBLOCK)
				g_string_append_printf(out, "\\begin{figure}[htbp]\n\\centering\n");

			g_string_append_printf(out, "\\includegraphics[");

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
			
			if ((height == NULL) && (width == NULL)) {
				/* No dimensions used */
				g_string_append_printf(out, "keepaspectratio,width=\\textwidth,height=0.75\\textheight");
			} else {
				/* At least one dimension given */
				if (!((height != NULL) && (width != NULL))) {
					/* we only have one */
					g_string_append_printf(out, "keepaspectratio,");
				}
				
				if (width != NULL) {
					if (width[strlen(width)-1] == '%') {
						width[strlen(width)-1] = '\0';
						temp_float = strtod(width, NULL);
						temp_float = temp_float/100;
						g_string_append_printf(out, "width=%.4f\\textwidth,", temp_float);
					} else {
						g_string_append_printf(out, "width=%s,",width);
					}
				} else {
					g_string_append_printf(out, "width=\\textwidth,");
				}
				
				if (height != NULL) {
					if (height[strlen(height)-1] == '%') {
						height[strlen(height)-1] = '\0';
						temp_float = strtod(height, NULL);
						temp_float = temp_float/100;
						g_string_append_printf(out, "height=%.4f\\textheight", temp_float);
					} else {
						g_string_append_printf(out, "height=%s",height);
					}
				} else {
					g_string_append_printf(out, "height=0.75\\textheight");
				}
			}

			g_string_append_printf(out, "]{%s}",n->link_data->source);
			
			if (n->key == IMAGEBLOCK) {
				if (n->children != NULL) {
					temp_str = g_string_new("");
					print_latex_node_tree(temp_str,n->children,scratch);
					if (temp_str->currentStringLength > 0) {
						g_string_append_printf(out, "\n\\caption{");
						g_string_append(out, temp_str->str);
						g_string_append_printf(out, "}");
					}
					g_string_free(temp_str, true);
				}
				
				if (n->link_data->label != NULL) {
					temp = label_from_string(n->link_data->label);
					g_string_append_printf(out, "\n\\label{%s}",temp);
					free(temp);
				}
				g_string_append_printf(out, "\n\\end{figure}");
				scratch->padded = 0;
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
			scratch->padded = 2;
			if (temp_node->key == GLOSSARYSOURCE) {
				g_string_append_printf(out, "\\newglossaryentry{%s}{",temp_node->children->children->str);
				print_latex_node_tree(out, temp_node->children, scratch);
				g_string_append_printf(out, "}}\\glsadd{%s}",temp_node->children->children->str);
			} else {
				g_string_append_printf(out, "\\footnote{");
				scratch->inside_footnote = true;
				print_latex_node_tree(out, temp_node->children, scratch);
				scratch->inside_footnote = false;
				g_string_append_printf(out, "}");
			}
			scratch->padded = 0;
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
					g_string_append_printf(out, "~\\nocite{%s}",&n->str[2]);
				} else {
					g_string_append_printf(out, "<FAKE span class=\"externalcitation\">");
					g_string_append_printf(out, "</span>");
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
						g_string_append_printf(out, "~\\nocite{%s}", n->link_data->label);
					} else {
						if (n->children != NULL) {
							g_string_append_printf(out, "~\\citep[");
							print_latex_node(out, n->children, scratch);
							g_string_append_printf(out,"]{%s}",n->link_data->label);
						} else {
							g_string_append_printf(out, "~\\citep{%s}", n->link_data->label);
						}
					}
				} else {
					/* not located -- this is external cite */
#ifdef DEBUG_ON
				fprintf(stderr, "no match for cite: '%s'\n",n->link_data->label);
#endif
					temp = n->link_data->label;
					if (n->key == NOCITATION) {
						g_string_append_printf(out, "~\\nocite{%s}",n->link_data->label);
					} else {
						if (n->children != NULL) {
#ifdef DEBUG_ON
				fprintf(stderr, "cite with children\n");
#endif
							if (strcmp(&temp[strlen(temp) - 1],";") == 0) {
								g_string_append_printf(out, " \\citet[");
								temp[strlen(temp) - 1] = '\0';
							} else {
								g_string_append_printf(out, "~\\citep[");
							}
							print_latex_node(out, n->children, scratch);
							g_string_append_printf(out, "]{%s}",temp);
						} else {
#ifdef DEBUG_ON
				fprintf(stderr, "cite without children. locat:'%s'\n",n->str);
#endif
							if (strcmp(&temp[strlen(temp) - 1],";") == 0) {
								temp[strlen(temp) - 1] = '\0';
								g_string_append_printf(out, " \\citet{%s}",temp);
							} else {
								g_string_append_printf(out, "~\\citep{%s}",temp);
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
				print_latex_string(out, temp, scratch);
				free(temp);
			}
			break;
		case GLOSSARYTERM:
			if ((n->next != NULL) && (n->next->key == GLOSSARYSORTKEY) ) {
				g_string_append_printf(out, "sort={");
				print_latex_string(out, n->next->str, scratch);
				g_string_append_printf(out, "},");
			}
			g_string_append_printf(out,"name={");
			print_latex_string(out, n->children->str, scratch);
			g_string_append_printf(out, "},description={");
			break;
		case GLOSSARYSORTKEY:
			break;
		case CODE:
			g_string_append_printf(out, "\\texttt{");
			print_latex_string(out, n->str, scratch);
			g_string_append_printf(out, "}");
			break;
		case BLOCKQUOTEMARKER:
			print_latex_node_tree(out, n->children, scratch);
			break;
		case BLOCKQUOTE:
			pad(out,2, scratch);
			g_string_append_printf(out, "\\begin{quote}");
			scratch->padded = 0;
			print_latex_node_tree(out, n->children, scratch);
			pad(out,1, scratch);
			g_string_append_printf(out, "\\end{quote}");
			scratch->padded = 0;
			break;
		case RAW:
			/* This shouldn't happen */
			g_string_append_printf(out, "RAW:");
			g_string_append_printf(out,"%s",n->str);
			break;
		case HTML:
			/* don't print HTML block */
			/* but do print HTML comments for raw LaTeX */
			if (strncmp(n->str,"<!--",4) == 0) {
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "%s", &n->str[4]);
				scratch->padded = 0;
			}
			break;
		case DEFLIST:
			pad(out,2, scratch);
			g_string_append_printf(out, "\\begin{description}");
			scratch->padded = 0;
			print_latex_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\end{description}");
			scratch->padded = 0;
			break;
		case TERM:
			pad(out,2, scratch);
			g_string_append_printf(out, "\\item[");
			print_latex_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "]");
			scratch->padded = 0;
			break;
		case DEFINITION:
			pad(out, 2, scratch);
			scratch->padded = 2;
			print_latex_node_tree(out, n->children, scratch);
			scratch->padded = 0;
			break;
		case TABLE:
			pad(out, 2, scratch);
			
			if (!scratch->inside_footnote)
				g_string_append_printf(out, "\\begin{table}[htbp]\n");

			g_string_append_printf(out,"\\begin{minipage}{\\linewidth}\n\\setlength{\\tymax}{0.5\\linewidth}\n\\centering\n\\small\n");
			print_latex_node_tree(out, n->children, scratch);

			g_string_append_printf(out, "\n\\end{tabulary}\n\\end{minipage}");

			if (!scratch->inside_footnote)
				g_string_append_printf(out, "\n\\end{table}");

			scratch->padded = 0;
			break;
		case TABLESEPARATOR:
			temp_str = g_string_new("");
			for (i = 0; n->str[i]; i++) {
				if (n->str[i] != 'h')
					g_string_append_printf(temp_str,"%c",toupper(n->str[i]));
			}
			g_string_append_printf(out, "\\begin{tabulary}{\\textwidth}{@{}%s@{}} \\toprule\n", temp_str->str);
			
			if (scratch->table_alignment != NULL)
				free(scratch->table_alignment);

			scratch->table_alignment = temp_str->str;
			g_string_free(temp_str, false);
			break;
		case TABLECAPTION:
			if ((n->children != NULL) && (n->children->key == TABLELABEL)) {
				temp = label_from_string(n->children->str);
			} else {
				temp = label_from_node_tree(n->children);
			}
			g_string_append_printf(out, "\\caption{");
			print_latex_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "}\n\\label{%s}\n", temp);
			free(temp);
			break;
		case TABLELABEL:
			break;
		case TABLEHEAD:
			print_latex_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\midrule\n");
			break;
		case TABLEBODY:
			print_latex_node_tree(out, n->children, scratch);
			if ((n->next != NULL) && (n->next->key == TABLEBODY)) {
				g_string_append_printf(out, "\n\\midrule\n");
			} else {
				g_string_append_printf(out, "\n\\bottomrule\n");
			}
			break;
		case TABLEROW:
			print_latex_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\\\\n");
			scratch->table_column = 0;
			break;
		case TABLECELL:
			scratch->padded = 2;
			temp = scratch->table_alignment;
			if ((n->children != NULL) && (n->children->key == CELLSPAN)) {
				g_string_append_printf(out, "\\multicolumn{%d}{%c}{",
					(int)strlen(n->children->str)+1,tolower(temp[scratch->table_column]));
			}
			print_latex_node_tree(out, n->children, scratch);
			if ((n->children != NULL) && (n->children->key == CELLSPAN)) {
				g_string_append_printf(out, "}");
			}
			if (n->next != NULL)
				g_string_append_printf(out, "&");
			if ((n->children != NULL) && (n->children->key == CELLSPAN)) {
				scratch->table_column += (int)strlen(n->children->str);
			}
			scratch->table_column++;
			break;
		case CELLSPAN:
			break;
		case GLOSSARYSOURCE:
			if (scratch->printing_notes)
				print_latex_node_tree(out, n->children, scratch);
			break;
		case CITATIONSOURCE:
		case NOTESOURCE:
			if (scratch->printing_notes)
				print_latex_node(out, n->children, scratch);
			break;
		case SOURCEBRANCH:
			fprintf(stderr,"SOURCEBRANCH\n");
			break;
		case NOTELABEL:
		case GLOSSARYLABEL:
			break;
		case SUPERSCRIPT:
			g_string_append_printf(out, "\\textsuperscript{%s}",n->str);
			break;
		case SUBSCRIPT:
			g_string_append_printf(out, "\\textsubscript{%s}",n->str);
			break;
		case KEY_COUNTER:
			break;
		case TOC:
			print_latex_node_tree(out,n->children, scratch);
			break;
		default:
			fprintf(stderr, "print_latex_node encountered unknown node key = %d\n",n->key);
			exit(EXIT_FAILURE);
	}
}

/* print_latex_endnotes */
void print_latex_endnotes(GString *out, scratch_pad *scratch) {
	scratch->used_notes = reverse_list(scratch->used_notes);
	scratch->printing_notes = 1;

	node *note = scratch->used_notes;
#ifdef DEBUG_ON
	fprintf(stderr, "start endnotes\n");
#endif
	
	if ((note == NULL) || ((note->key == KEY_COUNTER) && (note->next == NULL)))
		return;
	while (note != NULL) {
		if (note->key == CITATIONSOURCE)
			break;
		note = note->next;
	}

	if (note == NULL)
		return;
	
	note = scratch->used_notes;
	
	/* TODO: need CITATIONSOURCE to print bibliography */
#ifdef DEBUG_ON
	fprintf(stderr, "there are endnotes to print\n");
#endif

	pad(out, 2, scratch);
	g_string_append_printf(out, "\\begin{thebibliography}{0}");
	while ( note != NULL) {
		if (note->key == KEY_COUNTER) {
			note = note->next;
			continue;
		}
		
		pad(out, 1, scratch);
		
		if (note->key == CITATIONSOURCE) {
			g_string_append_printf(out, "\\bibitem{%s}\n", note->str);
			scratch->padded = 2;

			print_latex_node(out, note, scratch);
			pad(out, 1, scratch);
			scratch->padded = 1;
		} else {
			/* footnotes handled elsewhere */
		}

		note = note->next;
	}
	pad(out,2, scratch);
	g_string_append_printf(out, "\\end{thebibliography}");
	scratch->padded = 0;
#ifdef DEBUG_ON
	fprintf(stderr, "finish endnotes\n");
#endif
}

/* Check metadata keys and determine if we need a complete document */
/* We also preconvert metadata keys to proper formatting -- lowercase with no spaces */
bool is_latex_complete_doc(node *meta) {
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
			(strcmp(step->str, "xhtmlheader")      != 0) &&
			(strcmp(step->str, "htmlheader")       != 0) &&
			(strcmp(step->str, "quoteslanguage")   != 0))
			{ return TRUE;}
		step = step->next;
	}

	return FALSE;
}

/* print_latex_localized_typography -- convert to "smart" typography */
void print_latex_localized_typography(GString *out, int character, scratch_pad *scratch) {
	if (!extension(EXT_SMART, scratch->extensions)) {
		g_string_append_c(out, character);
		return;
	}
	switch (character) {
		case LSQUOTE:
			switch (scratch->language) {
				case SWEDISH:
					g_string_append_printf(out, "'");
					break;
				case FRENCH:
					g_string_append_printf(out,"'");
					break;
				case GERMAN:
					g_string_append_printf(out,"‚");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"›");
					break;
				default:
					g_string_append_printf(out,"`");
				}
			break;
		case RSQUOTE:
			switch (scratch->language) {
				case GERMAN:
					g_string_append_printf(out,"`");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"‹");
					break;
				default:
					g_string_append_printf(out,"'");
				}
			break;
		case APOS:
			g_string_append_printf(out,"'");
			break;
		case LDQUOTE:
			switch (scratch->language) {
				case DUTCH:
				case GERMAN:
					g_string_append_printf(out,"„");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"»");
					break;
				case FRENCH:
					g_string_append_printf(out,"«");
					break;
				case SWEDISH:
					g_string_append_printf(out, "''");
					break;
				default:
					g_string_append_printf(out,"``");
				}
			break;
		case RDQUOTE:
			switch (scratch->language) {
				case SWEDISH:
				case DUTCH:
					g_string_append_printf(out,"''");
					break;
				case GERMAN:
					g_string_append_printf(out,"``");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"«");
					break;
				case FRENCH:
					g_string_append_printf(out,"»");
					break;
				default:
					g_string_append_printf(out,"''");
				}
			break;
		case NDASH:
			g_string_append_printf(out,"--");
			break;
		case MDASH:
			g_string_append_printf(out,"---");
			break;
		case ELLIP:
			g_string_append_printf(out,"{\\ldots}");
			break;
			default:;
	}
}

/* print_latex_string - print string, escaping for LaTeX */
void print_latex_string(GString *out, char *str, scratch_pad *scratch) {
	char *tmp;
	char *start;
	if (str == NULL)
		return;
	start = str;	/* Store start of string */
	while (*str != '\0') {
		switch (*str) {
			case '{': case '}': case '$': case '%':
			case '&': case '_': case '#':
				g_string_append_printf(out, "\\%c", *str);
				break;
			case '^':
				g_string_append_printf(out, "\\^{}");
				break;
			case '\\':
				g_string_append_printf(out, "\\textbackslash{}");
				break;
			case '~':
				g_string_append_printf(out, "\\ensuremath{\\sim}");
				break;
			case '|':
				g_string_append_printf(out, "\\textbar{}");
				break;
			case '<':
				g_string_append_printf(out, "$<$");
				break;
			case '>':
				g_string_append_printf(out, "$>$");
				break;
			case '/':
				str++;
				while (*str == '/') {
					g_string_append_printf(out, "/");
					str++;
				}
				g_string_append_printf(out, "\\slash ");
				str--;
				break;
			case '\n':
				tmp = str;
				tmp--;
				if ((tmp > start) && (*tmp == ' ')) {
					tmp--;
					if (*tmp == ' ') {
						g_string_append_printf(out, "\\\\\n");
					} else {
						g_string_append_printf(out, "\n");
					}
				} else {
					g_string_append_printf(out, "\n");
				}
				break;
			case '-':
				str++;
				if (*str == '-') {
					g_string_append_printf(out, "-{}");
					str--;
				} else {
					str--;
					g_string_append_c(out,*str);
				}
				break;
			default:
				g_string_append_c(out, *str);
			}
		str++;
	}
}

/* print_latex_url - print url, escaping for LaTeX */
void print_latex_url(GString *out, char *str, scratch_pad *scratch) {
	if (str == NULL)
		return;
	while (*str != '\0') {
		switch (*str) {
			case '$': case '%': case '!':
			case '&': case '_': case '#':
				g_string_append_printf(out, "\\%c", *str);
				break;
			case '^':
				g_string_append_printf(out, "\\^{}");
				break;
			default:
				g_string_append_c(out, *str);
			}
		str++;
	}
}

char * correct_dimension_units(char *original) {
	char *result;
	int i;
	
	result = strdup(original);
	
	for (i = 0; result[i]; i++)
		result[i] = tolower(result[i]);
	
	if (strstr(&result[strlen(result)-2],"px")) {
		result[strlen(result)-2] = '\0';
		strcat(result, "pt");
	}
	
	return result;
}
