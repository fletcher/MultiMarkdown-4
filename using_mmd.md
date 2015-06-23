latex input:	mmd-tufte-handout-header
Title:	How to Use MultiMarkdown
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2015-06-23 
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


# How to Use MultiMarkdown #

There are several ways to use MultiMarkdown, depending on your needs. You can
use the `multimarkdown` command line tool, you can use MultiMarkdown with
several applications that support it directly, or you can use a drag and drop
approach.


## Command Line Usage ##

First, verify that you have properly installed MultiMarkdown:

	multimarkdown -v

If you don't see a message telling you which version of MultiMarkdown is
installed, check out Troubleshooting.

To learn more about the command line options to MultiMarkdown:

	multimarkdown -h


Once you have properly installed MultiMarkdown:

	multimarkdown file.txt

will convert the plain text file `file.txt` into HTML output. To save the
results to a file:

	multimarkdown file.txt > file.html

A shortcut to this is to use MultiMarkdown's batch mode, which will save the
output to the same base filename that is input, with the extension `.html` (or
`.tex` for LaTeX output):

	multimarkdown -b file.txt

A benefit of batch mode is that you can process multiple files at once:

	multimarkdown -b file1.txt file2.txt file3.txt

If you want to create LaTeX output instead of HTML:

	multimarkdown -t latex file.txt

For LyX:

	multimarkdown -t lyx file.txt

For OPML:

	multimarkdown -t opml file.txt

For RTF (RTF output is limited -- check the output carefully to be sure it's ok for your needs):

	multimarkdown -t rtf file.txt

And for an OpenDocument text file:

	multimarkdown -t odf file.txt

If you are using "basic" transclusion (not dependent on particular output formats), you can
use MMD to perform the transclusion and output the raw MMD source:

	multimarkdown -t mmd file.txt

There are also several convenience scripts included with MultiMarkdown:

	mmd file.txt
	mmd2tex file.txt
	mmd2opml file.txt
	mmd2odf file.txt

These scripts run MultiMarkdown in batch mode to generate HTML, LaTeX, OPML,
or ODF files respectively. These scripts are included with the Mac or Windows
installers, and are available for *nix in the `scripts` directory in the
source project. They are intended to be used as shortcuts for the most common
command line options.


### Command Line Options ##

There are several options when running MultiMarkdown from the command line.

	multimarkdown -h, multimarkdown --help

This shows a summary of how to use MultiMarkdown.

	multimarkdown -v, multimarkdown --version

Displays the version of MultiMarkdown currently installed.

	multimarkdown -o, multimarkdown --output=FILE

Directs the output to the specified file. By default, the output is directed
to `stdout`. The use of `batch` mode obviates the need to use this option, but
if you want to specify a different output filename it can be handy.

	multimarkdown -t html|latex|memoir|beamer|opml|odf|rtf|lyx|lyx-beamer

This options specified the format that MultiMarkdown outputs. The default is
html. If you use the `LaTeX Mode` metadata, then MultiMarkdown will
automatically choose `memoir` or `beamer` as directed without using these
command line options. Using that metadata will also allow the various
convenience scripts to choose the correct output format as well.

	multimarkdown -b, multimarkdown --batch

Automatically redirects the output to a file with the same base name as the
input file, but with the appropriate extension based on the output type. For
example, `multimarkdown -b file.txt` would output the HTML to `file.html`, and
`multimarkdown -b -t latex file.txt` would output to `file.tex`.

	multimarkdown -c, multimarkdown --compatibility

Compatibility mode causes MultiMarkdown to output HTML that is compatible
with that output from the original Markdown. This allows it to pass the
original Markdown test suite. Syntax features that don't exist in regular
Markdown will still be output using the regular MultiMarkdown output
formatting.

	multimarkdown -f, multimarkdown --full

The `full` option forces a complete document, even if it does not contain enough metadata to otherwise trigger a complete document. 

	multimarkdown -s, multimarkdown --snippet

The `snippet` option forces the output of a "snippet", meaning that header and footer information is left out.  This means that a LaTeX document might not have enough information to be processed, for example.

	multimarkdown --process-html

This option tells MultiMarkdown to process the text included within HTML tags
in the source document.  This can feature can also be implemented on a tag-by-tag
basis within the document itself, such as `<div markdown="1">`.

	multimarkdown -m, multimarkdown --metadata-keys

List all of the available metadata keys contained in a document, one key per line.

	multimarkdown -e "metakey", multimarkdown --extract "metakey"

The extract feature outputs the value of the specified metadata key. This is
used in my convenience scripts to help choose the proper LaTeX output mode,
and could be used in other circumstances as well.

	multimarkdown --random

Tell MultiMarkdown to use random identifier numbers for footnotes.  Useful when you might combine multiple HTML documents together, e.g. in a weblog.

	multimarkdown --accept
	multimarkdown --reject
	multimarkdown --accept --reject

Tell MultiMarkdown whether to accept or reject changes in written in
[CriticMarkup] format within the document.  Use both together if you 
want to highlight the differences -- this only works for HTML output.

	multimarkdown --smart
	multimarkdown --nosmart

Tell MultiMarkdown whether to use "smart" typography, similar to John Gruber's
[SmartyPants](http://daringfireball.net/projects/smartypants/) program, which
was included in MultiMarkdown 2.0. This extension is turned on by default in
MultiMarkdown.


	multimarkdown --notes
	multimarkdown --nonotes

Tell MultiMarkdown whether to use footnotes (enabled by default).

	multimarkdown --labels
	multimarkdown --nolabels

Tell MultiMarkdown whether to add id attributes to headers in HTML (enabled by default).

	multimarkdown --mask
	multimarkdown --nomask

Tell MultiMarkdown whether to mask email addresses when creating HTML (enabled by default).

	multimarkdown --notes

Enables the use of footnotes and similar markup (glossary, citations). Enabled
by default in MultiMarkdown.

Other options are available by checking out `multimarkdown --help-all`, but
the ones listed above are the primary options.


### Advanced Mode ###

MultiMarkdown version 2.0 had to first convert the source file to HTML, and
then applied XSLT files to convert to the final LaTeX format. Since
MultiMarkdown 3.0 can create LaTeX directly, this approach is no longer
necessary.

The one benefit of that approach, however, was that it became possible to
perform a wide range of customizations on exactly how the LaTeX output was
created by customizing the XSLT files.

If you install the Support files on Mac or Linux, you can still use the
advanced XSLT method to generate LaTeX output. For the time being, this
approach doesn't work with Windows, but it would be fairly easy to create a
batch script or perl script to implement this feature on Windows.

Keep in mind, however, that because of the more advanced mechanism of handling
LaTeX in MultiMarkdown 3.0, you can do a great deal of customization without
needing to use an XSLT file.

The `mmd2tex-xslt` script will convert a plain text file into LaTeX that is
virtually identical with that created by the regular LaTeX approach.

There are a few differences in the two approaches, however:

* Once a MultiMarkdown file is converted to HTML, it is impossible to tell
  whether the resulting HTML was generated by MultiMarkdown, or if it was
  included as raw HTML within the source document. So *either* way, it will
  be converted to the analagous LaTeX syntax. The `multimarkdown` binary on
  its own will *not* convert HTML into LaTeX.

* The whitespace that is generated will be different under certain
  circumstances. Typically, this will result in one extra or one fewer blank
  lines with the the XSLT approach. Generally this will not be an issue, but
  when used with `<!-- some comment -->` it may cause a newline to be lost.

* The default XSLT recognizes `class="noxslt"` when applied to HTML entities,
  and will discard them from the output.

* An XSLT can only be applied to a complete HTML document, not a "snippet".
  Therefore, if you want to use the XSLT method, your file must have metadata
  that triggers a complete document (i.e. any metadata except "quotes
  language" or "base header level").

* Using XSL to process an HTML file will "de-obfuscate" any email addresses
  that were obfuscated by MultiMarkdown.


### Recommendations ###

I recommend that you become familiar with the "basic" approach to using
MultiMarkdown before trying to experiment with XSLT. The basic approach is
faster, and easier, and the results can still be customized quite a bit.

Then you can experiment with modifying XSLT to further customize your output
as needed.

If you have XSLT files that you used in MultiMarkdown 2.0, you will likely
need to modify them to recognize the HTML output generated by MultiMarkdown
3.0. You can use the default XSLT files as a guide to what is different.


## Mac OS X Applications ##

There are several applications that have built-in support for MultiMarkdown,
or that can easily use it with a plug-in.

### Using MultiMarkdown With MultiMarkdown Composer ###

[MultiMarkdown Composer] is my commercial text editor designed from the ground up around the MultiMarkdown (and Markdown) syntax.  It contains a great deal of features to make writing, editing, and exporting MultiMarkdown documents easier than ever before.  I certainly recommend it, but since I created it, and it's not free, you may believe me to biased.  So search the internet to see what people are saying, then check it out.

### Using MultiMarkdown with TextMate ###

If you want to run MultiMarkdown from directly within [TextMate], you should
install my MultiMarkdown [bundle]. This is a modified version of the original
Markdown bundle for TextMate that includes better support for MultiMarkdown.

This bundle will work with MultiMarkdown 2, or with MultiMarkdown 3/4 if you
install the Mac Support Installer files (available from the downloads
[page][download]).


### Using MultiMarkdown with Scrivener ###


[Scrivener] is a great program for writers using Mac OS X. It includes built
in support for MultiMarkdown. If you want to use MultiMarkdown 3/4 with
Scrivener, you need to install the Support files in `~/Library/Application
Support/MultiMarkdown`. The Mac Support Installer is available from
the downloads [page][download] and will install these files for you.


### Drag and Drop ###

You can use the Mac OS X drag and drop applications to allow you to convert
MultiMarkdown to other formats by dragging and dropping files in the Finder.
They are available from the [download] page, or by running `make drop` from
the command line in the `multimarkdown` source directory.


### MultiMarkdown and Finder "Quick Look" ###

Starting in Mac OS 10.5, the Finder has the ability to show a "Quick Look"
preview of the contents of a file. I have a Quick Look generator that allows
the Finder to preview the contents of a MultiMarkdown text file (or OPML file)
as an HTML preview.

I recommend using the latest (closed-source) version available for [download][composer-download].  It contains advanced features that are not available in the open source version.

Source code for the older version is available for download from [github](https://github.com/fletcher/MMD-QuickLook).


## Using MultiMarkdown in Windows ##

You can use the same command line approach with Windows as described
previously. While there aren't drag and drop applications per se for the
Windows system, you can use Windows Explorer to create links to the binary and
specify and desired command line options to change the default output format.
This will effectively allow you to create drag and drop applications for
Windows.


## MultiMarkdown and LaTeX ##

Of note [LaTeX] is a complex set of programs. MultiMarkdown doesn't include
LaTeX in the installer --- it's up to the user to install a working LaTeX
setup on their machine if you want to use it.

What MultiMarkdown does is make it easier to generate documents using the
LaTeX syntax. It should handle 80% of the documents that 80% of MultiMarkdown
need. It doesn't handle all circumstances, and sometimes you will need to hand
code your LaTeX yourself.

In those cases you have a few options. MultiMarkdown will pass text included
in HTML comments along to the LaTeX as raw output. For example:

	<!--  This is raw \LaTeX \[ {e}^{i\pi }+1=0 \] -->

You can also include your desired LaTeX code in a separate file and link to
it:

	<!-- \input{somefile} -->

If you have questions about LaTeX itself, I can't help. You're welcome to send
your question to the MultiMarkdown [discussion list], and perhaps someone will
be able to offer some assistance. But you would be better off asking a group
dedicated to LaTeX instead.

If the problem is that MultiMarkdown itself is generating invalid LaTeX, then
of course I want to know about it so I can fix it.

If you need more information about how to use LaTeX to process a file into a
PDF, check out the [faq](#process-latex).


## MultiMarkdown and OPML ##

MultiMarkdown is well suited to plain text files, but it can also be useful to
work on MultiMarkdown documents in an outliner or mind-mapping application.
For this, it is easy to convert back and forth between OPML and plain text
MultiMarkdown.

To convert from a text file to OPML:

	multimarkdown -t opml -b file.txt

or:

	mmd2opml file.txt

The resulting OPML file uses the headings to build the outline structure, and
puts the text within each section as a not for the corresponding level of the
outline using the `_note` attribute. **NOTE**: not all outliners support this
attribute. On Mac OS X, [OmniOutliner]
(http://www.omnigroup.com/applications/omnioutliner/) is a fabulous outliner
that supports this field. If you're into mind mapping software, [iThoughts]
(http://www.ithoughts.co.uk/) works on the iPad/iPhone and supports import and
export with OPML and the `_note` attribute.

To convert from OPML, you can use various commands in from the [MMD-Support]
(https://github.com/fletcher/MMD-Support) package:

	opml2HTML file.opml
	opml2mmd file.opml
	opml2LaTeX file.opml

**NOTE**: These scripts require a working installation of `xsltproc`, and the
ability to run shell scripts. This should work by default on most
installations of Mac OS X or Linux, but will require these applications to be
installed separately on Windows.


## MultiMarkdown and OpenDocument ##

It is also possible to convert a MultiMarkdown text file into a word
processing document for [OpenOffice.org](http://www.openoffice.org/) or
[LibreOffice](http://www.libreoffice.org/download). This file can then be
converted by one of those applications into RTF, or a Microsoft Word document,
or many other file formats. (If you're not familiar with these applications,
they are worth checking out. I don't understand why people use Microsoft
Office any more...)

	multimarkdown -b -t odf file.txt

or

	mmd2odf file.txt

MultiMarkdown 2.0 had partial support for outputting an RTF file, and could do
it completely on Mac OS X by using Apple's `textutil` program. MMD 3 no longer
directly supports RTF as an output format, but the Flat OpenDocument format is
a much better option.

**NOTE**: LibreOffice can open these Flat OpenDocument files by default, but
OpenOffice requires that you install the `OpenDocument-Text-Flat-XML.jar` file
available from the
[downloads](https://github.com/fletcher/peg-multimarkdown/downloads) page. To
install it, create a new document in OpenOffice (or open an existing one),
then go to the Tools->XML Filter Settings menu option. Use the "Open
Package..." button to import the downloaded `.jar` file.

## MultiMarkdown and RTF ##

I have made it clear in various places that RTF is a horrible format for sharing documents.  Seriously -- it's really bad.

That said, MultiMarkdown now offers direct conversion to RTF documents (sort of).  This export format is not complete.  Tables don't work very well, and lists don't work properly.  Images are not supported.

If you have a very simple document, this may work just fine.

If you have a more complex document, I encourage you to use the OpenDocument export, and to use [LibreOffice] instead of a commercial Word-processor (you know what I'm talking about).  Even if you use LibreOffice to convert your OpenDocument to RTF, you'll get better results.

## MultiMarkdown and LyX ##

[LyX] is is a document processor that seems to be a sort of hybrid between a markup language processor and a word processor.  I'll be honest -- I don't quite get it, and I don't use it.

That said, Charles Cowan has contributed code to the MultiMarkdown project that enables exporting of LyX documents directly.  If you have any trouble getting this to work, please use the MultiMarkdown [issues page](https://github.com/fletcher/MultiMarkdown-4/issues) to get help.

See [his page](http://crcowan.github.io/MultiMarkdown-4-LyX-Maintenance/) for more information.

*Note*: Because the LyX exporter is not maintained by me, it may take some time for new features to be supported when exporting to LyX.

### Advanced Use ###

It is possible to use an XSLT file to customize the OpenDocument output from
MultiMarkdown. I suppose you could also write an XSLT to convert OpenDocument
into LaTeX, similar to the default ones that convert HTML into LaTeX.

You can also create an XSLT that converts the OpenDocument output and modifies
it to incorporate necessary customizations. While a little tricky to learn,
XSLT files can be quite powerful and you're limited only by your imagination.


### Limitations ###

There are several limitations to the OpenDocument Flat Text format:

* images are not fully supported --- they work best if you specify a length
  and a width in "fixed" units (not '%'), or do not specify any dimensions.

* citations are not supported --- I would like to be able to do something
  here, but I suspect you will need to use an external tool for the time
  being

* math features are not supported, though I hope to be able to implement this
  at some point in the future
