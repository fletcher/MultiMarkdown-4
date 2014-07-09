latex input:	mmd-tufte-handout-header
Title:	Glossaries
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-07-09  
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Glossaries ##

MultiMarkdown has a feature that allows footnotes to be specified as glossary
terms. It doesn't do much for XHTML documents, but the XSLT file that converts
the document into LaTeX is designed to convert these special footnotes into
glossary entries.

The glossary format for the footnotes is:

	[^glossaryfootnote]: glossary: term (optional sort key)
		The actual definition belongs on a new line, and can continue on
		just as other footnotes.

The `term` is the item that belongs in the glossary. The `sort key` is
optional, and is used to specify that the term should appear somewhere else in
the glossary (which is sorted in alphabetical order).

Unfortunately, it takes an extra step to generate the glossary when creating a
pdf from a latex file:

1. You need to have the `basic.gst` file installed, which comes with the
memoir class.

2. You need to run a special makeindex command to generate the `.glo` file:
		``makeindex -s `kpsewhich basic.gst` -o "filename.gls" "filename.glo"``

3. Then you run the usual pdflatex command again a few times.

Alternatively, you can use the code below to create an engine file for TeXShop
(it belongs in `~/Library/TeXShop/Engines`). You can name it something like
`MemoirGlossary.engine`. Then, when processing a file that needs a glossary,
you typeset your document once with this engine, and then continue to process
it normally with the usual LaTeX engine. Your glossary should be compiled
appropriately. If you use [TeXShop][], this is the way to go.

**Note**: *Getting glossaries to work is a slightly more advanced LaTeX
feature, and might take some trial and error the first few times.*


	#!/bin/	
	
	set path = ($path /usr/local/teTeX/bin/powerpc-apple-darwin-current 
		/usr/local/bin) # This is actually a continuation of the line above
	
	set basefile = `basename "$1" .tex`
	
	makeindex -s `kpsewhich basic.gst` -o "${basefile}.gls" "${basefile}.glo"
