latex input:	mmd-tufte-handout-header  
Title:	Hacking MultiMarkdown  
Author:	Fletcher T. Penney  
Version:	{{version.txt}}  
Revised:	2014-10-01  
Base Header Level:	3  
CSS:	{{css.txt}}  
MMD Footer:	links.txt  
latex mode:	memoir  
latex input:	mmd-tufte-handout-begin-doc  
latex footer:	mmd-tufte-footer  


# "Hacking" MultiMarkdown #

## Multiple Formats ##

On some occasions, the same MultiMarkdown syntax is not ideal for different output formats.  For example, a link may need to be slightly different for the HTML version than for the LaTeX version (since LaTeX offers the `\autoref()` feature).

You have a few options for crafting MultiMarkdown that will be handled differently for HTML than for another format.

1. You can use HTML comments to include text that is ignored in HTML documents, but is passed through verbatim to other formats.  See the section on [Raw Source][rawhtml] for more information.  This only works with one format besides HTML.

2. You can use file transclusion with [Wildcard Extensions] to embed a different string of text for each output format.  This requires a bit more organization, but allows you to do just about anything you like.
 