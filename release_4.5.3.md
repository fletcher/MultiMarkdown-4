latex input:  mmd-tufte-handout-header
Title:  Version 4.5.3 Release Notes
Author: Fletcher T. Penney
Version:  {{version.txt}}
Revised:  2015-03-04  
Base Header Level:  2
CSS:  {{css.txt}}
MMD Footer: meta-links.txt
latex mode: memoir
latex input:  mmd-tufte-handout-begin-doc
latex footer: mmd-tufte-footer

## 4.5.3 ##

* Include mmd2rtf in Windows installer
* improved support for inline footnotes
* Fix potential bug when handling beamer latexmode metadata
* add `latex title` and `latex author` metadata support
* Fix bug when sequential dashes used in code spans for LaTeX export (Thanks to Thomas Hodgson for noticing this)
* Add to list of known HTML5 tags
* update windows batch scripts
* Improve metadata handling in snippet mode
* Improved transclusion path logic
* Fix order of citations when printing footnotes in HTML
* Fix potential memory leaks
