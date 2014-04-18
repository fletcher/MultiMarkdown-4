latex input:	mmd-article-header
Title:	Smart Typography
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-04-18  
Base Header Level:	1
CSS:	{{css.txt}}
HTML header:	<script type="text/javascript"
	src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
	</script>
latex mode:	memoir
latex input:	mmd-article-begin-doc
latex footer:	mmd-memoir-footer


## Smart Typography ##

MultiMarkdown incorporates John Gruber's [SmartyPants](http://daringfireball.net/projects/smartypants/) tool in addition to the core Markdown functionality.  This program converts "plain" punctuation into "smarter" typographic punctuation.

Just like the original, MultiMarkdown converts:

* Straight quotes (`"` and `'`) into "curly" quotes 
* Backticks-style quotes (` ``this'' `) into "curly" quotes
* Dashes (`--` and `---`) into en- and em- dashes
* Three dots (`...`) become an ellipsis

MultiMarkdown also includes support for quotes styles other than English (the default).  Use the `quotes language` metadata to choose:

* dutch (`nl`)
* german(`de`)
* germanguillemets
* french(`fr`)
* swedish(`sv`)

This feature is enabled by default, but is disabled in `compatibility` mode, since it is not part of the original Markdown.  You can also use the `smart` and `nosmart` command line options to control this feature.