latex input:	mmd-tufte-handout-header
Title:	Smart Typography
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-04-18  
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


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