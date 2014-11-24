Title:	MultiMarkdown 4 ReadMe  
Author:	Fletcher T. Penney  
Base Header Level:	2  

Introduction
===============

[Markdown] is a simple markup language used to convert plain text into HTML. 

[MultiMarkdown] is a derivative of Markdown that adds new syntax features, such as footnotes, tables, and metadata. Additionally, it offers mechanisms to convert plain text into LaTeX in addition to HTML. 


## Background ##

MultiMarkdown started as a Perl script, which was modified from the original Markdown.pl.

MultiMarkdown v3 (aka 'peg-multimarkdown') was based on John MacFarlane's [peg-markdown].  It used a parsing expression grammar (PEG), and was written in C in order to compile on almost any operating system.  Thanks to work by Daniel Jalkut, MMD v3 was built so that it didn't have any external library requirements.

MultiMarkdown v4 is basically a complete rewrite of v3.  It uses the same basic PEG for parsing (Multi)Markdown text, but otherwise is almost completely rebuilt:

* The code is designed to be easier to maintain --- it's divided into separate files on a more logical structure
* All memory leaks (to my knowledge) have been fixed
* [greg] is used instead of [peg/leg] to create the parser --- this allows the parser to be thread-safe
* The [test suite] has been modified to account for several improvements.  MMD should fail one of the basic Markdown tests (see [peg-markdown] for more information).
* Command line options are *slightly* different.


# Installation #

You can compile for yourself, or download a precompiled binary from the [downloads] page.

* Download the source from the [github] web site using `git`
* `git submodule init` and then `git submodule update` to download `greg` and the test suite
* Run `make` to compile.
* Run `make test-all | less` to verify that the build is correct.  As mentioned, one of the tests is expected to fail; the rest should pass on all systems.  
* Run `sudo make install` and (optionally) `sudo make install-scripts` to install `multimarkdown` and the helper scripts.


# Usage #

Once installed, you simply do something like the following: 

* `multimarkdown file.txt` --- process text into HTML. 

* `multimarkdown -c file.txt` --- use a compatibility mode that emulates the original Markdown. 

* `multimarkdown -t latex file.txt` --- output the results as LaTeX instead of HTML. This can then be processed into a PDF if you have LaTeX installed. You can further specify the `LaTeX Mode` metadata to customize output for compatibility with `memoir` or `beamer` classes. 

* `multimarkdown -t odf file.txt` --- output the results as an OpenDocument Text Flat XML file. Does require the plugin be installed in your copy of OpenOffice, which is available at the [peg-multimarkdown-downloads] page. LibreOffice includes this plugin by default. 

* `multimarkdown -t opml file.txt` --- convert the MMD text file to an MMD OPML file, compatible with OmniOutliner and certain other outlining and mind-mapping programs (including iThoughts and iThoughtsHD). 

* `multimarkdown -h` --- display help and additional options. 

* `multimarkdown -b *.txt` --- `-b` or `--batch` mode can process multiple files at once, converting `file.txt` to `file.html` or `file.tex` as directed. Using this feature, you can convert a directory of MultiMarkdown text files into HTML files, or LaTeX files with a single command without having to specify the output files manually. **CAUTION**: This will overwrite existing files with the `html` or `tex` extension, so use with caution. 


# Notes #

If you get an error that `greg` fails to build try `touch greg/greg.c`.  I had an issue where the timestamp on that file might have been too old, which caused the build to fail.


# LyX Support #

Charles R. Cowan (<https://github.com/crcowan>) added support for conversion to [LyX](http://www.lyx.org/).  Support for this should be considered to be in alpha/beta, and is not guaranteed.  Issues related to LyX can be added to the MultiMarkdown [issues] page on github, but will need to be answered by Charles.  I am happy to include this code in the main MMD repo, but since I don't use LyX I can't support it myself.  If this arrangement becomes a problem, then LyX support can be removed and it can be kept as a separate fork.

# More Information #

To get more information about MultiMarkdown, check out the [website][MultiMarkdown] or [User's Guide].

[peg-markdown]:	https://github.com/jgm/peg-markdown
[Markdown]:	http://daringfireball.net/projects/markdown/
[MultiMarkdown]:	http://fletcherpenney.net/multimarkdown/
[peg-multimarkdown]:	https://github.com/fletcher/peg-multimarkdown
[peg-multimarkdown-downloads]:	https://github.com/fletcher/peg-multimarkdown/downloads
[fink]:	http://www.finkproject.org/
[downloads]:	http://fletcherpenney.net/multimarkdown/download/
[GTK+]:	http://www.gtk.org/
[homebrew]:	https://github.com/mxcl/homebrew
[MacPorts]:	http://www.macports.org/
[test suite]:	https://github.com/fletcher/MMD-Test-Suite
[github]:	https://github.com/fletcher/MultiMarkdown-4
[greg]:	https://github.com/nddrylliog/greg
[peg/leg]:	http://piumarta.com/software/peg/
[issues]:	https://github.com/fletcher/MultiMarkdown-4/issues
[User's Guide]:	http://fletcher.github.io/MultiMarkdown-4/
