latex input:  mmd-tufte-handout-header
Title:  Metadata
Author: Fletcher T. Penney
Version:  {{version.txt}}
Revised:  2014-04-18 
Base Header Level:  2
CSS:  {{css.txt}}
MMD Footer: meta-links.txt
latex mode: memoir
latex input:  mmd-tufte-handout-begin-doc
latex footer: mmd-tufte-footer


## Metadata ##

It is possible to include special metadata at the top of a MultiMarkdown
document, such as title, author, etc. This information can then be used to
control how MultiMarkdown processes the document, or can be used in certain
output formats in special ways.  For example:

```
{{examples/metadata.text}}
```

The syntax for including metadata is simple.

* The metadata must begin at the very top of the document - no blank lines can 	precede it.  There can optionally be a `---` on the line before and after the metadata.  The line after the metadata can also be `...`.  This is to provide better compatibility with [YAML], though MultiMarkdown doesn't support all YAML metadata.

* Metadata consists of the two parts - the `key` and the `value`

* The metadata key must begin at the beginning of the line. It must start with
  an ASCII letter or a number, then the following characters can consist of ASCII letters, numbers, spaces, hyphens, or underscore characters.

* The end of the metadata key is specified with a colon (':')

* After the colon comes the metadata value, which can consist of pretty much
  any characters (including new lines). To keep multiline metadata values from
  being confused with additional metadata, I recommend indenting each new line
  of metadata. If your metadata value includes a colon, it *must* be indented
  to keep it from being treated as a new key-value pair.

* While not required, I recommend using two spaces at the end of each line of
  metadata. This will improve the appearance of the metadata section if your
  document is processed by Markdown instead of MultiMarkdown.

* Metadata keys are case insensitive and stripped of all spaces during
  processing. This means that `Base Header Level`, `base headerlevel`, and
  `baseheaderlevel` are all the same.

* Metadata is processed as plain text, so it should *not* include
  MultiMarkdown markup. It is possible to create customized XSLT files that
  apply certain processing to the metadata value, but this is not the default
  behavior.

* After the metadata is finished, a blank line triggers the beginning of the
  rest of the document.

## Metadata "Variables" ##

You can substitute the `value` for a metadata `key` in the body of a document using the following format, where `foo` and `bar` are the `key`s of the desired metadata.

```
{{examples/metadata-variable.text}}
```

## "Standard" Metadata keys ##

There are a few metadata keys that are standardized in MultiMarkdown. You can
use any other keys that you desire, but you have to make use of them yourself.

My goal is to keep the list of "standard" metadata keys as short as possible.


### Author ###

This value represents the author of the document and is used in LaTeX, ODF, and RTF
documents to generate the title information.


### Affiliation ###

This is used to enter further information about the author --- a link to a
website, the name of an employer, academic affiliation, etc.


### Base Header Level ###

This is used to change the top level of organization of the document.  For example:

	Base Header Level: 2
	
	# Introduction #

Normally, the Introduction would be output as `<h1>` in HTML, or `\part{}` in
LaTeX. If you're writing a shorter document, you may wish for the largest
division in the document to be `<h2>` or `\chapter{}`. The `Base Header Level`
metadata tells MultiMarkdown to change the largest division level to the
specified value.

This can also be useful when combining multiple documents.

`Base Header Level` does not trigger a complete document.

Additionally, there are "flavors" of this metadata key for various output
formats so that you can specify a different header level for different output
formats --- e.g. `LaTeX Header Level`, `HTML Header Level`, and `ODF Header
Level`.

If you are doing something interesting with [File Transclusion], you can also use a
negative number here.  Since metadata is not used when a file is "transcluded",
this allows you to use a different level of headings when a file is processed on
its own.


### Biblio Style ###

This metadata specifies the name of the BibTeX style to be used, if you are
not using natbib.


### BibTeX ###

This metadata specifies the name of the BibTeX file used to store citation
information. Do not include the trailing '.bib'.


### Copyright ###

This can be used to provide a copyright string.


### CSS ###

This metadata specifies a URL to be used as a CSS file for the produced
document. Obviously, this is only useful when outputting to HTML.


### Date ###

Specify a date to be associated with the document.


### HTML Header ###

You can include raw HTML information to be included in the header.
MultiMarkdown doesn't perform any validation on this data --- it just copies
it as is.

As an example, this can be useful to link your document to a working MathJax
installation (not provided by me):

	HTML header:  <script type="text/javascript"
		src="http://example.net/mathjax/MathJax.js">
		</script>


### Quotes Language ###

This is used to specify which style of "smart" quotes to use in the output document.  The available options are:

* dutch (or `nl`)
* english 
* french (`fr`)
* german (`de`)
* germanguillemets
* swedish (`sv`)

The default is `english` if not specified. This affects HTML output. To
change the language of a document in LaTeX is up to the individual.

`Quotes Language` does not trigger a complete document.


### LaTeX Author ###

Since MultiMarkdown syntax is not processed inside of metadata, you can use the `latex author` metadata to override the regular author metadata when exporting to LaTeX.

This metadata *must* come after the regular `author` metadata if it is also being used.

### LaTeX Footer ###

A special case of the `LaTeX Input` metadata below. This file will be linked
to at the very end of the document.


### LaTeX Input ###

When outputting a LaTeX document it is necessary to include various directions
that specify how the document should be formatted. These are not included in
the MultiMarkdown document itself --- instead they should be stored separately
and linked to with `\input{file}` commands.

These links can be included in the metadata section. The metadata is processed
in order, so I generally break my directives into a group that need to go
before my metadata, a group that goes after the metadata but before the
document itself, and a separate group that goes at the end of the document,
for example:

	latex input:		mmd-memoir-header
	Title:				MultiMarkdown Example
	Base Header Level:	2
	latex mode:			memoir
	latex input:		mmd-memoir-begin-doc
	latex footer:		mmd-memoir-footer

You can download the [LaTeX Support Files] if you want to output documents
using the default MultiMarkdown styles. You can then use these as examples to
create your own customized LaTeX output.

This function should allow you to do almost anything you could do using the
XSLT features from MultiMarkdown 2.0. More importantly, it means that advanced
LaTeX users do not have to learn XSLT to customize their code as desired.


### LaTeX Mode ###

When outputting a document to LaTeX, there are two special options that change
the output slightly --- `memoir` and `beamer`. These options are designed to
be compatible with the LaTeX classes of the same names.


### LaTeX Title ###

Since MultiMarkdown syntax is not processed inside of metadata, you can use the `latex title` metadata to override the regular title metadata when exporting to LaTeX.

This metadata *must* come after the regular `title` metadata if it is also being used.


### MMD Footer ###

The `MMD Footer` metadata is used to specify the name of a file that should be appended to the end of the document using the [File Transclusion] feature.  This is useful for keeping a list of references, abbreviations, footnotes, links, etc. all in a single file that can be reused across multiple documents.  If you're building a big document out of smaller documents, this allows you to use one list in all files, without multiple copies being inserted in the master file.


### ODF Header ###

You can include raw XML to be included in the header of a file output in
OpenDocument format. It's up to you to properly format your XML and get it
working --- MultiMarkdown just copies it verbatim to the output.


### Title ###

Self-explanatory.


### Transclude Base ###

When using the [File Transclusion] feature to "link" to other documents inside a MultiMarkdown document, this metadata specifies a folder that contains the files being linked to.  If omitted, the default is the folder containing the file in question.  This can be a relative path or a complete path.

This metadata can be particularly useful when using MultiMarkdown to parse a text string that does not exist as a file on the computer, and therefore does not have a parent folder (when using `stdin` or another application that offers MultiMarkdown support).  In this case, the path must be a complete path.
