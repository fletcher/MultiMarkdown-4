latex input:	mmd-article-header  
Title:	Basic Concepts of MultiMarkdown  
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-01-04 
Base Header Level:	1
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-article-begin-doc
latex footer:	mmd-memoir-footer

# Basic Concepts of MultiMarkdown #

## How do I create a MultiMarkdown document? ##

The general concept in MultiMarkdown is that it should be easy for someone to
type a plain text file that is human-readable, and then use the MultiMarkdown
program to convert that text file into a more complicated computer language
such as HTML or LaTeX. This allows you to create high quality output without
having to spend hours and hours fiddling with font sizes, margins, etc.

The first step in learning to use MultiMarkdown is to learn how to use
[Markdown]. MultiMarkdown is an
extension to Markdown, and builds off of the basic fundamentals used in
Markdown.

I recommend starting by familiarizing yourself with the Markdown [basics] and
[syntax] pages.

Once you're familiar with the basics of Markdown, it will be relatively easy
to pick up the advanced features included in MultiMarkdown.

[basics]: http://daringfireball.net/projects/markdown/basics
[syntax]: http://daringfireball.net/projects/markdown/syntax


## Complete documents vs "snippets" ##

In order to include metadata information such as a title, the HTML document
created by MultiMarkdown must be "complete." This means that it starts with
something that looks like (for HTML):

	<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
	<!DOCTYPE html>
	<html xmlns="http://www.w3.org/1999/xhtml">
	<head>

If you include metadata in your document (with a few exceptions), then you will
generate a complete document. If you don't include metadata, then you will
instead generate a "snippet." The snippet will just include the relevant
portion of HTML, but will not include the `<head>` or `<body>` elements.

Metadata that is only intended to affect the way MultiMarkdown processes the
output will not trigger a complete document. Currently, this means you can use
`Base Header Level` or `Quotes Language` and still output a snippet if you
don't include any other metadata.

Additionally, if you include (via [Transclusion]) a file that contains MultiMarkdown metadata, the metadata is stripped before being inserted into the parent document.  This is slightly different than the idea of a snippet, but the effect of ignoring the metadata is similar.
