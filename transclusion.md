latex input:	mmd-tufte-handout-header
Title:	File Transclusion
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2015-06-23  
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## File Transclusion ##

File transclusion is the ability to tell MultiMarkdown to insert the contents of another file inside the current file being processed.  For example:

	This is some text.
	
	{{some_other_file.txt}}
	
	Another paragraph

If a file named `some_other_file.txt` exists, its contents will be inserted inside of this document *before* being processed by MultiMarkdown.  This means that the contents of the file can also contain MultiMarkdown formatted text.

If you want to display the *contents* of the file without processing it, you can include it in a code block (you may need to remove trailing newlines at the end of the document to be included):

	This is some text

	```
	{{relative/path/to/some_other_file.txt}}
	```

	Another paragraph

Transclusion is recursive, so the file being inserted will be scanned to see if it references any other files.

Metadata in the file being inserted will be ignored.  This means that the file can contain certain metadata when viewed alone that will not be included when the file is transcluded by another file.

You can use the `[Transclude Base]` metadata to specify where MultiMarkdown should look for the files to be included.  All files must be in this folder.  If this folder is not specified, then MultiMarkdown will look in the same folder as the parent file.

**Note:**  Thanks to David Richards for his ideas in developing support for this feature.

### Wildcard Extensions ###

Sometimes you may wish to transclude alternate versions of a file depending on your output format.  Simply use the extension ".*" to have MMD choose the proper version of the file (e.g. `foo.tex`, `foo.fodt`, `foo.html`, etc.)

	Insert a different version of a file here based on export format:
	{{foo.*}}


### Transclusion Preprocessing ###

If you want to perform transclusion, *without* converting to another format, you can use `mmd` as the output format:

	multimarkdown -t mmd foo.txt

This will only perform "basic" transclusion --it does not support wildcard extensions, since the final output format is not known.
