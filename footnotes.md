latex input:	mmd-article-header
Title:	Footnotes
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2013-12-31 
Base Header Level:	1
CSS:	{{css.txt}}
latex mode:	memoir
latex input:	mmd-article-begin-doc
latex footer:	mmd-memoir-footer

I have added support for footnotes to MultiMarkdown, using the syntax proposed
by John Gruber. Note that there is no official support for footnotes yet, so
the output format may change, but the input format sounds fairly stable.

To create a footnote, enter something like the following:

    Here is some text containing a footnote.[^somesamplefootnote]
	
    [^somesamplefootnote]: Here is the text of the footnote itself.
	
    [somelink]:http://somelink.com


The footnote itself must be at the start of a line, just like links by
reference. If you want a footnote to have multiple paragraphs, lists, etc.,
then the subsequent paragraphs need an extra tab preceding them. You may have
to experiment to get this just right, and please let me know of any issues you
find.

This is what the final result looks like:

> Here is some text containing a footnote.[^somesamplefootnote]

[^somesamplefootnote]: Here is the text of the footnote itself.

{{links.txt}}
