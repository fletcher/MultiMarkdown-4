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


## Scrivener Tricks ##

[Scrivener] is a full-featured tool for writers that includes some support for MultiMarkdown when exporting to other formats.

### Multiple Citations ###

When using Scrivener to publish to LaTeX, you may want to include multiple sources within a single citation.  Mike Thicke suggests this approach:

	In the Compile | Replacements dialog I have:
	
	][],[# --> ,
	],[][# --> ,
	
	So for citations like this:
	
	[][#Tversky:1974wi],[][#Kahneman:1979wl],[][#Tversky:1981vc]
	
	I get this:
	
	[][#Tversky:1974wi,Kahneman:1979wl,Tversky:1981vc]
	
	When complied to Latex it becomes:
	
	 ~\citep{Tversky:1974wi,Kahneman:1979wl,Tversky:1981vc} 
	And finally:
	
	(Tversky and Kahneman 1974; Kahneman and Tversky 1979; Tversky and Kahneman 1981)
	
	If you want page numbers you might have to do raw latex or use RegEx replacements.

You could always doing something similar with a script to find/replace the same syntax in tools other than Scrivener.
