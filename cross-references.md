latex input:	mmd-tufte-handout-header
Title:	Cross-References
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-01-04 
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Cross-References ##

An oft-requested feature was the ability to have Markdown automatically handle
within-document links as easily as it handled external links. To this aim, I
added the ability to interpret `[Some Text][]` as a cross-link, if a header
named "Some Text" exists.

As an example, `[Metadata][]` will take you to the
[section describing metadata][Metadata].

Alternatively, you can include an optional label of your choosing to help
disambiguate cases where multiple headers have the same title:

	### Overview [MultiMarkdownOverview] ##

This allows you to use `[MultiMarkdownOverview]` to refer to this section
specifically, and not another section named `Overview`. This works with atx-
or settext-style headers.

If you have already defined an anchor using the same id that is used by a
header, then the defined anchor takes precedence.

In addition to headers within the document, you can provide labels for images
and tables which can then be used for cross-references as well.
