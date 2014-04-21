latex input:	mmd-tufte-handout-header
Title:	Known Issues
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-01-04 
Base Header Level:	3
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


# Known Issues #

## OpenDocument ##

### OpenDocument doesn't properly support image dimensions ###

It's relatively easy to insert an image into ODF using fixed dimensions, but
harder to get a scaled image without knowing the exact aspect ratio of the
image.

For example, in LaTeX or HTML, one can specify that image should be scaled to
50% of the width, and have it automatically calculate the proper height. This
does not work in ODF, at least not that I can find.

You have to manually adjust the image to fit your desired constraint. It's
easy to do, simply hold down the shift key while adjusting the image size, and
it will likely snap to match the specified dimension.

I welcome suggestions on a better way to do this.


## RTF ##

* Non-ASCII characters are not supported
* Lists are not proper lists
* Images are not supported
* Tables are not fully supported

## OPML ##

### OPML doesn't handle "skipped" levels ###

When converting a MMD text file to OPML with the mmd binary, each level only
contains it's direct children. For example:

	# First Level #

	## Second Level  ##

	### Third Level ###

	## Another Second Level ##

	#### Fourth Level ####

When this is converted to OPML, the "Fourth Level" item will be deleted, since it skips a level from its parent, "Another Second Level".

It's possible to fix this, but it's going to take a more complicated algorithm than what I currently have and it's not a high priority for me to fix at the moment.

As always, suggestions welcome.
