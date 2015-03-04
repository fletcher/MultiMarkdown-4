latex input:  mmd-tufte-handout-header
Title:  Version 4.7 Release Notes
Author: Fletcher T. Penney
Version:  {{version.txt}}
Revised:  2015-03-04  
Base Header Level:  2
CSS:  {{css.txt}}
MMD Footer: meta-links.txt
latex mode: memoir
latex input:  mmd-tufte-handout-begin-doc
latex footer: mmd-tufte-footer

## 4.7 ##

* As of version 4.7, the OS X MultiMarkdown installer will no longer support
	ppc processors.  You will have to compile yourself for these machines.
* `{{TOC}}` inserts a basic Table of Contents inside the document
* Improve abbreviation matching to include blockquotes, lists
* `mmd` export format -- performs transclusions but doesn't parse
* restructure test suite
* add a `cmake` experimental branch -- same source, different build system
* fix error with certain invalid image links
* update documentation
* don't break if heading immediately follows table
* fix error when parsing multiple files at once
* allow for raw RTF
* add `mmd_header` metadata support
* improve code blocks in beamer (thanks to marco-m)
* improve backtick style quotes
* improve transclusion with nested directories
* option to show manifest of transcluded files (`-x` or `--manifest`)
* Improved YAML support
* Update `greg`
* Improved Unicode support on Windows when transcluding
* Strip BOM when transcluding
* Support "japanese commas" for underscored strong/emph
* show "--manifest" option in help
* Other bug fixes
