latex input:	mmd-tufte-handout-header
Title:	Fenced Code Blocks
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2015-02-20 
Base Header Level:	2
CSS:	{{css.txt}}
html header:	<link rel="stylesheet" href="http://yandex.st/highlightjs/7.3/styles/default.min.css">
	<script src="http://yandex.st/highlightjs/7.3/highlight.min.js"></script>
	<script>hljs.initHighlightingOnLoad();</script>
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Fenced Code Blocks ##

In addition to the regular indented code block that Markdown uses, you can use "fenced" code blocks in MultiMarkdown.  These code blocks do not have to be indented, and can also be configured to be compatible with a third party syntax highlighter.  These code blocks should begin with 3 to 5 backticks, an optional language specifier (if using a syntax highlighter), and should end with the same number of backticks you started with:

```perl
# Demonstrate Syntax Highlighting if you link to highlight.js #
# http://softwaremaniacs.org/soft/highlight/en/
print "Hello, world!\n";
$a = 0;
while ($a < 10) {
print "$a...\n";
$a++;
}
```

I don't recommend any specific syntax highlighter, but have used the following metadata to set things up.  It may or may not work for you:

```
html header:	<link rel="stylesheet" href="http://yandex.st/highlightjs/7.3/styles/default.min.css">
	<script src="http://yandex.st/highlightjs/7.3/highlight.min.js"></script>
	<script>hljs.initHighlightingOnLoad();</script>
```

Fenced code blocks are particularly useful when including another file ([File Transclusion]), and you want to show the *source* of the file, not what the file looks like when processed by MultiMarkdown.
