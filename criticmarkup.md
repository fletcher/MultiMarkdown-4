latex input:	mmd-article-header  
Title:	CriticMarkup  
Author:	Fletcher T. Penneys
Version:	{{version.txt}}
Revised:	2014-01-04 
Base Header Level:	1
CSS:	{{css.txt}}
latex mode:	memoir
latex input:	mmd-article-begin-doc
latex footer:	mmd-memoir-footer

# CriticMarkup #

## What Is CriticMarkup? ##

> CriticMarkup is a way for authors and editors to track changes to documents in plain text. As with Markdown, small groups of distinctive characters allow you to highlight insertions, deletions, substitutions and comments, all without the overhead of heavy, proprietary office suites. <http://criticmarkup.com/>

CriticMarkup is integrated with MultiMarkdown itself, as well as [MultiMarkdown Composer].  I encourage you to check out the web site to learn more as it can be a very useful tool.  There is also a great video showing CriticMarkup in use while editing a document in MultiMarkdown Composer.

## The CriticMarkup Syntax ##

The CriticMarkup syntax is fairly straightforward.  The key thing to remember is that CriticMarkup is processed *before* any other MultiMarkdown is handled.  It's almost like a separate layer on top of the MultiMarkdown syntax.

When editing in MultiMarkdown Composer, you can have CriticMarkup syntax flagged in the both the editor pane and the preview window.  This will allow you to see changes in the HTML preview.

When using CriticMarkup with MultiMarkdown itself, you have three choices:

* Leave the CriticMarkup syntax in place
* Accept all changes, giving you the "new" document (`multimarkdown -a foo.txt`)
* Reject all changes, giving you the "original" document (`multimarkdown -r foo.txt`)

CriticMarkup comments and highlighting are ignored when processing.

Deletions from the original text:

	This is {--is --}a test.

Additions:

	This {++is ++}a test.

Substitutions:

	This {~~isn't~>is~~} a test.

Highlighting:

	This is a {==test==}.

Comments:

	This is a test{>>What is it a test of?<<}.

{{links.txt}}