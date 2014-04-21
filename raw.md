latex input:	mmd-tufte-handout-header
Title:	Raw Source
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-01-04 
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Raw HTML ##

You can include raw (X)HTML within your document. Exactly what happens with
these portions depends on the output format. You can also use the `markdown`
attribute to indicate that MultiMarkdown processing should be applied within
the block level HTML tag. This is in addition to the `--process-html` command
line option that causes MultiMarkdown processing to occur within *all* block
level HTML tags.

For example:

	<div>This is *not* MultiMarkdown</div>
	
	<div markdown=1>This *is* MultiMarkdown</div>

will produce the following without `--process-html`:

	<div>This is *not* MultiMarkdown</div>
	
	<div>This <em>is</em> MultiMarkdown</div>

and with `--process-html`:

	<div>This is <em>not</em> MultiMarkdown</div>
	
	<div>This <em>is</em> MultiMarkdown</div>


However, the results may be different than anticipated when outputting to
LaTeX or other formats. Normally, block level HTML will be ignored when
outputting to LaTeX or ODF. The example above would produce the following,
leaving out the first `<div>` entirely:

	This \emph{is} MultiMarkdown

And this with `--process-html`:

	This is \emph{not} MultiMarkdown
	This \emph{is} MultiMarkdown

You will also notice that the line breaks are different when outputting to
LaTeX or ODF, and this can cause the contents of two `<div>` tags to be placed
into a single paragraph.


## Raw LaTeX/OpenDocument/etc. ##

You can use HTML comments to include additional text that will be included in the exported file without being changed.  This can be used for any export format, which means that each document can only be configured for one export format at a time.  In other words,  it is highly unlikely that valid raw LaTeX will also be valid OpenDocument source code.

	This will be processed by *MultiMarkdown*.
	<!-- This will not be processed by *MultiMarkdown -->
