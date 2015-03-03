/*

	rtf.c -- RTF writer

	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "rtf.h"


/* Since RTF sucks, we simplify here */

#define kNormalStyle "\\s0 \\qj\\sa180\\f0\\fs24 "
#define kH1 "\\s1 \\f1\\fs32\\ql\\sb240\\sa180\\b "
#define kH2 "\\s2 \\f1\\fs28\\ql\\sb240\\sa180\\i\\b "
#define kH3 "\\s3 \\f1\\fs28\\ql\\sb240\\sa180\\b "
#define kH4 "\\s4 \\f1\\fs24\\ql\\sb240\\sa180\\i\\b "
#define kH5 "\\s5 \\f1\\fs24\\ql\\sb240\\sa180\\b "
#define kH6 "\\s6 \\f1\\fs22\\ql\\sb240\\sa180\\b "
#define kQuoteStyle "\\s7 \\qj\\sa180\\f0\\fs24\\li720\\ri720 "
#define kNoteStyle "\\s7 \\qj\\sa180\\f0\\fs24\\li360\\ri360 "
#define kCodeStyle "\\s7 \\qj\\sa180\\f2\\fs20\\li360\\ri360 "

/* print_rtf_node_tree -- convert node tree to RTF */
void print_rtf_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_rtf_node(out, list, scratch);
		list = list->next;
	}
}

/* print_rtf_node -- convert given node to RTF and append */
void print_rtf_node(GString *out, node *n, scratch_pad *scratch) {
	int i;
	int lev;
	int old_type;
	char *temp;
	link_data *temp_link_data;
	node *temp_node;

	switch (n->key) {
		case SPACE:
		case STR:
		/* TODO: Some of the following need improvements */
		case MATHSPAN:
			print_rtf_string(out, n->str, scratch);
			break;
		case METADATA:
			if (scratch->extensions & EXT_SNIPPET)
				break; 			
			g_string_append_printf(out, "{\\info\n");
			print_rtf_node_tree(out,n->children,scratch);
		 	g_string_append_printf(out, "}\n");
			scratch->padded = 0;
			break;
		case METAKEY:
			/* Convert key */
			temp = label_from_string(n->str);
			free(n->str);
			n->str = temp;
			if (strcmp(n->str, "baseheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
				break;
			} else if (strcmp(n->str, "rtfheaderlevel") == 0) {
				scratch->baseheaderlevel = atoi(n->children->str);
				break;
			} else if (strcmp(n->str, "quoteslanguage") == 0) {
				temp = label_from_node_tree(n->children);
				if ((strcmp(temp, "nl") == 0) || (strcmp(temp, "dutch") == 0)) { scratch->language = DUTCH; }   else 
				if ((strcmp(temp, "de") == 0) || (strcmp(temp, "german") == 0)) { scratch->language = GERMAN; } else 
				if (strcmp(temp, "germanguillemets") == 0) { scratch->language = GERMANGUILL; } else 
				if ((strcmp(temp, "fr") == 0) || (strcmp(temp, "french") == 0)) { scratch->language = FRENCH; } else 
				if ((strcmp(temp, "sv") == 0) || (strcmp(temp, "swedish") == 0)) { scratch->language = SWEDISH; }
				free(temp);
				break;
			}
	
			if (strcmp(n->str, "title") == 0) {
				g_string_append_printf(out, "{\\title ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "author") == 0) {
				g_string_append_printf(out, "{\\author ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "affiliation") == 0) {
				g_string_append_printf(out, "{\\company ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "company") == 0) {
				g_string_append_printf(out, "{\\company ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "keywords") == 0) {
				g_string_append_printf(out, "{\\keywords ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "copyright") == 0) {
				g_string_append_printf(out, "{\\*\\copyright ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "comment") == 0) {
				g_string_append_printf(out, "{\\doccomm ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			} else if (strcmp(n->str, "subject") == 0) {
				g_string_append_printf(out, "{\\subject ");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out, "}\n");
			}
			break;
		case METAVALUE:
			trim_trailing_whitespace(n->str);
			print_rtf_string(out, n->str, scratch);
			break;
		case BLOCKQUOTEMARKER:
			print_rtf_node_tree(out, n->children, scratch);
			break;
		case BLOCKQUOTE:
			old_type = scratch->odf_para_type;
			scratch->odf_para_type = BLOCKQUOTE;
			pad_rtf(out, 2, scratch);
			print_rtf_node_tree(out,n->children,scratch);
			scratch->padded = 1;
			scratch->odf_para_type = old_type;
			break;
		case VERBATIM:
		case VERBATIMFENCE:
			pad_rtf(out, 2, scratch);
			g_string_append_printf(out, "{\\pard " kCodeStyle);
			print_rtf_code_string(out,n->str,scratch);
			g_string_append_printf(out, "\n\\par}\n");
			scratch->padded = 0;
			break;
		case CODE:
			print_rtf_node_tree(out,n->children,scratch);
//			print_rtf_string(out, n->str, scratch);
			break;
		case PARA:
			pad_rtf(out, 2, scratch);
			switch (scratch->odf_para_type) {
				case BLOCKQUOTE:
					g_string_append_printf(out, "{\\pard " kQuoteStyle);
					break;
				case NOTEREFERENCE:
				case CITATION:
					g_string_append_printf(out, "{\\pard " kNoteStyle);
					break;
				case CODE:
				case VERBATIM:
				case VERBATIMFENCE:
					g_string_append_printf(out, "{\\pard " kCodeStyle);
					break;
				default:
					g_string_append_printf(out, "{\\pard " kNormalStyle);
					break;
			}
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\n\\par}\n");
			scratch->padded = 1;
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;
			if (lev > 6)
				lev = 6;
			pad_rtf(out, 2, scratch);
			switch (lev) {
				case 1:
					g_string_append_printf(out, "{\\pard " kH1);
					break;
				case 2:
					g_string_append_printf(out, "{\\pard " kH2);
					break;
				case 3:
					g_string_append_printf(out, "{\\pard " kH3);
					break;
				case 4:
					g_string_append_printf(out, "{\\pard " kH4);
					break;
				case 5:
					g_string_append_printf(out, "{\\pard " kH5);
					break;
				case 6:
					g_string_append_printf(out, "{\\pard " kH6);
					break;
			}
			if (n->children->key == AUTOLABEL) {
				temp = label_from_string(n->children->str);
				g_string_append_printf(out, "{\\*\\bkmkstart %s}{\\*\\bkmkend %s}",temp, temp);
				print_rtf_node_tree(out, n->children->next, scratch);
			} else {
				temp = label_from_node_tree(n->children);
				g_string_append_printf(out, "{\\*\\bkmkstart %s}{\\*\\bkmkend %s}",temp,temp);
				print_rtf_node_tree(out, n->children, scratch);
			}
			free(temp);
			g_string_append_printf(out, "\\par}\n");
			scratch->padded = 1;
			break;
		case TABLE:
			if ((n->children != NULL) && (n->children->key == TABLECAPTION)) {
				if (n->children->children->key == TABLELABEL) {
					temp = label_from_string(n->children->children->str);
				} else {
					temp = label_from_node_tree(n->children->children);
				}
				g_string_append_printf(out, "{\\*\\bkmkstart %s}{\\*\\bkmkend %s}",temp,temp);
				free(temp);
			}
			pad_rtf(out, 2, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			if ((n->children != NULL) && (n->children->key == TABLECAPTION)) {
				g_string_append_printf(out, "{\\pard " kNormalStyle "\\qc ");
				print_rtf_node_tree(out, n->children->children, scratch);
				g_string_append_printf(out, "\\par}\n");
			}
			g_string_append_printf(out, "\\pard\\par\n");
			scratch->padded = 1;
			break;
		case TABLELABEL:
		case TABLECAPTION:
			break;
		case TABLESEPARATOR:
			scratch->table_alignment = n->str;
			break;
		case TABLEHEAD:
			scratch->cell_type = 'h';
			print_rtf_node_tree(out, n->children, scratch);
			scratch->cell_type = 'd';
			break;
		case TABLEROW:
			scratch->table_column = 0;
			g_string_append_printf(out, "\\trowd\\trautofit1\n");
			for (i=0; i < strlen(scratch->table_alignment); i++) {
				g_string_append_printf(out, "\\cellx%d\n",i+1);
			}
			print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\row\n");
			break;
		case TABLECELL:
			temp = scratch->table_alignment;
			if (strncmp(&temp[scratch->table_column],"h",1) == 0) {
				scratch->table_column++;
			}
			lev = scratch->table_column;
	
			g_string_append_printf(out, "\\intbl");

			if (scratch->cell_type == 'h') {
				g_string_append_printf(out, "\\qc{\\b ");
			} else {
				if ( strncmp(&temp[lev],"r",1) == 0) {
					g_string_append_printf(out, "\\qr");
				} else if ( strncmp(&temp[lev],"R",1) == 0) {
					g_string_append_printf(out, "\\qr");
				} else if ( strncmp(&temp[lev],"c",1) == 0) {
					g_string_append_printf(out, "\\qc");
				} else if ( strncmp(&temp[lev],"C",1) == 0) {
					g_string_append_printf(out, "\\qc");
				} else {
					g_string_append_printf(out, "\\ql");
				}
			}
			g_string_append_printf(out, " {");
			print_rtf_node_tree(out, n->children, scratch);

			if (scratch->cell_type == 'h')
				g_string_append_printf(out, "}");

			g_string_append_printf(out, "}\\cell\n");
			scratch->table_column++;
			break;
		case STRONG:
			g_string_append_printf(out, "{\\b ");
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "}");
			break;
		case EMPH:
			g_string_append_printf(out, "{\\i ");
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "}");
			break;
		case LINEBREAK:
			g_string_append_printf(out, "\\line ");
			break;
		case LINK:
			temp_link_data = load_link_data(n, scratch);

			if (temp_link_data == NULL) {
				/* replace original text since no definition found */
				g_string_append_printf(out, "[");
				print_rtf_node(out, n->children, scratch);
				g_string_append_printf(out,"]");
				if (n->children->next != NULL) {
					g_string_append_printf(out, "[");
					print_rtf_node_tree(out, n->children->next, scratch);
					g_string_append_printf(out,"]");
				} else if (n->str != NULL) {
					/* no title label, so see if we stashed str*/
					g_string_append_printf(out, "%s", n->str);
				} else {
					g_string_append_printf(out, "[%s]",n->link_data->label);
				}

				free_link_data(temp_link_data);
				break;
			}

			/* Insert link */
			g_string_append_printf(out, "{\\field{\\*\\fldinst{HYPERLINK \"");
			print_rtf_string(out, temp_link_data->source, scratch);
			g_string_append_printf(out, "\"}}{\\fldrslt ");
			if (n->children != NULL)
				print_rtf_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "}}");

			free(temp_link_data);
			break;
		case BULLETLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\ls1\\ilvl0 ");
			scratch->padded = 0;
			print_rtf_node_tree(out, n->children, scratch);
			break;
		case ORDEREDLIST:
			pad(out, 2, scratch);
			scratch->padded = 0;
			print_rtf_node_tree(out, n->children, scratch);
			break;
		case LISTITEM:
			g_string_append_printf(out, "{\\listtext \\'95 }");
			print_rtf_node_tree(out, n->children, scratch);
			break;
		case NOTEREFERENCE:
			lev = note_number_for_node(n, scratch);
			temp_node = node_for_count(scratch->used_notes, lev);
			scratch->padded = 2;

			g_string_append_printf(out, "{\\super\\chftn}{\\footnote\\pard\\plain\\chtfn ");
			print_rtf_node_tree(out, temp_node->children, scratch);
			g_string_append_printf(out, "}");
			scratch->padded = 0;
			break;
		case GLOSSARYTERM:
			print_rtf_string(out, n->children->str, scratch);
			g_string_append_printf(out, ": ");
			break;
		case GLOSSARYSORTKEY:
			break;
		case NOCITATION:
		case CITATION:
			if ((n->link_data != NULL) && (strncmp(n->link_data->label,"[#",2) == 0)) {
				/* external citation */
				g_string_append_printf(out, "%s", n->link_data->label);
			} else {
				/* MMD citation, so output as endnote */
				scratch->printing_notes = 1;
				lev = 0;
				if (n->link_data != NULL)
					lev = note_number_for_label(n->link_data->label, scratch);
				if (lev != 0) {
					temp_node = node_for_count(scratch->used_notes, lev);
					
					/* flag that this is used as a citation */
					temp_node->key = CITATIONSOURCE;
					if (lev > scratch->max_footnote_num) {
						/* first use of this citation */
						scratch->max_footnote_num = lev;
						
						old_type = scratch->odf_para_type;
						scratch->odf_para_type = CITATION;
						
						/* change to represent cite count only */
						// lev = cite_count_node_from_end(temp_node);
						g_string_append_printf(out, "{\\super\\chftn}{\\footnote\\ftnalt\\pard\\plain\\chtfn ");
						scratch->padded = 2;
						if (temp_node->children != NULL) {
							print_rtf_node(out, temp_node->children, scratch);
						}
						pad(out, 1, scratch);
						g_string_append_printf(out, "}");
						scratch->odf_para_type = old_type;
					} else {
						/* We are reusing a previous citation */

						/* Change lev to represent cite count only */
						// lev = cite_count_node_from_end(temp_node);
					
						g_string_append_printf(out, "REUSE CITATION");
					}
				} else {
					/* not located -- this is external cite */

					if ((n->link_data != NULL) && (n->key == NOCITATION)) {
						g_string_append_printf(out, "%s", n->link_data->label);
					} else if (n->link_data != NULL) {
						g_string_append_printf(out, "[");
						if (n->children != NULL) {
							print_rtf_node(out, n->children, scratch);
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
			break;
		case APOSTROPHE:
			print_rtf_localized_typography(out, APOS, scratch);
			break;
		case ELLIPSIS:
			print_rtf_localized_typography(out, ELLIP, scratch);
			break;
		case EMDASH:
			print_rtf_localized_typography(out, MDASH, scratch);
			break;
		case ENDASH:
			print_rtf_localized_typography(out, NDASH, scratch);
			break;
		case SINGLEQUOTED:
			print_rtf_localized_typography(out, LSQUOTE, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			print_rtf_localized_typography(out, RSQUOTE, scratch);
			break;
		case DOUBLEQUOTED:
			print_rtf_localized_typography(out, LDQUOTE, scratch);
			print_rtf_node_tree(out, n->children, scratch);
			print_rtf_localized_typography(out, RDQUOTE, scratch);
			break;
		case LIST:
		case HEADINGSECTION:
			print_rtf_node_tree(out,n->children,scratch);
			break;
		/* TODO: Some of the following need improvements */
		case TABLEBODY:
		case PLAIN:
			print_rtf_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\\\n");
			break;
		case NOTELABEL:
		case GLOSSARYLABEL:
		case FOOTER:
		case LINKREFERENCE:
			break;
		case GLOSSARYSOURCE:
		case CITATIONSOURCE:
		case NOTESOURCE:
			if (scratch->printing_notes)
				print_html_node_tree(out, n->children, scratch);
			break;
		case IMAGEBLOCK:
		case IMAGE:
			g_string_append_printf(out, "IMAGES CANNOT BE INSERTED INTO AN RTF DOCUMENT FROM MULTIMARKDOWN \\\n");
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
		case HTMLBLOCK:
			/* don't print HTML block */
			/* but do print HTML comments for raw RTF */
			if (strncmp(n->str,"<!--",4) == 0) {
				pad(out, 2, scratch);
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "%s", &n->str[4]);
				scratch->padded = 0;
			}
			break;
		case TOC:
			print_rtf_node_tree(out,n->children, scratch);
			break;
		default:
			fprintf(stderr, "print_rtf_node encountered unknown node key = %d\n",n->key);
			g_string_append_printf(out, "%s",n->str);
			/* Will use in place of real output during development */
			/* exit(EXIT_FAILURE); */
			break;
	}
}

void begin_rtf_output(GString *out, node* list, scratch_pad *scratch) {
	g_string_append_printf(out, "{\\rtf1\\ansi\\deff0 {\\fonttbl\n" \
		"{\\f0\\froman Times New Roman;}\n" \
		"{\\f1\\fswiss Arial;}\n" \
		"{\\f2\\fmodern Courier New;}\n" \
		"}\n" \
		"{\\stylesheet\n" \
		"{" kNormalStyle "Normal;}\n" \
		"{" kH1 "Header 1;}\n" \
		"{" kH2 "Header 2;}\n" \
		"{" kH3 "Header 3;}\n" \
		"{" kH4 "Header 4;}\n" \
		"{" kH5 "Header 5;}\n" \
		"{" kH6 "Header 6;}\n" \
		"{" kQuoteStyle "Quotation;}\n" \
		"{" kNoteStyle "Note;}\n" \
		"}\n" \
		"\\margt1150\\margb1150\\margl1150\\margr1150\n");
}

void end_rtf_output(GString *out, node* list, scratch_pad *scratch) {
	g_string_append_printf(out, "}\n");}

/* print_rtf_localized_typography -- convert to "smart" typography */
void print_rtf_localized_typography(GString *out, int character, scratch_pad *scratch) {
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
					g_string_append_printf(out,"\\'91");
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
					g_string_append_printf(out,"\\'92");
				}
			break;
		case APOS:
			g_string_append_printf(out,"\\'27");
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
					g_string_append_printf(out,"\\'93");
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
					g_string_append_printf(out,"\\'94");
				}
			break;
		case NDASH:
			g_string_append_printf(out,"\\'96");
			break;
		case MDASH:
			g_string_append_printf(out,"\\'97");
			break;
		case ELLIP:
			g_string_append_printf(out,"\\'85");
			break;
			default:;
	}
}

void print_rtf_string(GString *out, char *str, scratch_pad *scratch) {
	if (str == NULL)
		return;
	while (*str != '\0') {
		switch (*str) {
			case '\\':
				g_string_append_printf(out, "\\\\");
				break;
			case '{':
				g_string_append_printf(out, "\\{");
				break;
			case '}':
				g_string_append_printf(out, "\\}");
				break;
			case '\n':
				g_string_append_printf(out, " \n");
			default:
				g_string_append_c(out, *str);
		}
		str++;
	}
}

void print_rtf_code_string(GString *out, char *str, scratch_pad *scratch) {
	if (str == NULL)
		return;
	while (*str != '\0') {
		switch (*str) {
			case '\\':
				g_string_append_printf(out, "\\\\");
				break;
			case '{':
				g_string_append_printf(out, "\\{");
				break;
			case '}':
				g_string_append_printf(out, "\\}");
				break;
			case '\n':
				g_string_append_printf(out, "\\\n");
				break;
			default:
				g_string_append_c(out, *str);
		}
		str++;
	}
}
void pad_rtf(GString *out, int num, scratch_pad *scratch) {
	while (num-- > scratch->padded)
		g_string_append_printf(out, "\n");
	
	scratch->padded = num;
}
