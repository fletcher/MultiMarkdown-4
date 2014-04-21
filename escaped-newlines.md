latex input:	mmd-tufte-handout-header
Title:	Escaped Newlines
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-02-05 
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Escaped newlines ##

Thanks to a contribution from [Nicolas](https://github.com/njmsdk), MultiMarkdown has an additional syntax to indicate a line break.  The usual approach for Markdown is "space-space-newline" --- two spaces at the end of the line.  For some users, this causes problems:

* the trailing spaces are typically invisible when glancing at the source, making it easy to overlook them
* some users' text editors modify trailing space (IMHO, the proper fix for this is a new text editor...)

Nicolas submitted a patch that enables a new option that interprets "\\" before a newline as a marker that a line break should be used:

	This is a line.\
	This is a new line.

To enable this feature, use the following option:

	multimarkdown --escaped-line-breaks file.txt

If this option is not enabled, then the default behavior will be to treat the newline as an escaped character, which results in it simply appearing as a newline character in the output.  This means that the default behavior is the same as if the "\\" is not in the source file.
