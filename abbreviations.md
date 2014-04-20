latex input:	mmd-tufte-handout-header
Title:	Abbreviations
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-04-19 
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Abbreviations ##

MultiMarkdown includes support for abbreviations, as implemented in Michel Fortin's [PHP Markdown Extra](http://michelf.ca/projects/php-markdown/extra/). Basically, you define an abbreviation using the following syntax:

	*[HTML]: HyperText Markup Language
	*[W3C]:  World Wide Web Consortium

Then, wherever you use the words `HTML` or `W3C` in your document, the `abbr` markup will be added:

	The HTML specification
	is maintained by the W3C.

becomes:

	The <abbr title="Hyper Text Markup Language">HTML</abbr> specification
	is maintained by the <abbr title="World Wide Web Consortium">W3C</abbr>.

Here's an example using HTML and W3C.  The exact behavior will depend on which format you are viewing this document in.  Especially if we use HTML and W3C again. (Remember that HTML has probably already been used if you're viewing a longer version of this document.)

*[HTML]: HyperText Markup Language
*[W3C]:  World Wide Web Consortium


As in PHP Markdown Extra, abbreviations are case-sensitive and will work on multiple word abbreviations.  In this case, MultiMarkdown is tolerant of different variations of whitespace between words.

	Operation Tigra Genesis is going well.
	
	*[Tigra Genesis]:

An abbreviation with an empty definition results in an omitted `title` attribute.

There are a few limitations:

* The full name of the abbreviation is plain text only -- no MultiMarkdown markup will be processed.
* Abbreviations don't do anything when exporting to ODF -- there's not an equivalent structure there -- it would have to be hand coded.  I may or may not get around to this, but pull requests welcome.  ;)
* When exporting to LaTeX, the `acronym` package is used; this means that the first usage will result in `full text (short)`, and subsequent uses will result in `short`.