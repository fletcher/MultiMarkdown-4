latex input:	mmd-article-header
Title:	File Transclusion
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2013-12-31 
Base Header Level:	1
CSS:	{{css.txt}}
latex mode:	memoir
latex input:	mmd-article-begin-doc
latex footer:	mmd-memoir-footer

## File Transclusion ##

File transclusion is the ability to tell MultiMarkdown to insert the contents of another file inside the current file being processed.  For example:

	This is some text.
	
	{{some_other_file.txt}}
	
	Another paragraph

If a file named `some_other_file.txt` exists, its contents will be inserted inside of this document *before* being processed by MultiMarkdown.  This means that the contents of the file can also MultiMarkdown.

If you want to display the *contents* of the file without processing it, you can include it in a code block (you may need to remove trailing newlines at the end of the document to be included):

	This is some text

	```
	{{relative/path/to/some_other_file.txt}}
	```

	Another paragraph

Transclusion is recursive, so the file being inserted will be scanned to see if it references any other files.

Metadata in the file being inserted will be ignored.  This means that the file can contain certain metadata when viewed alone that will not be included when the file is transcluded by another file.

You can use the [Transclude Base] metadata to specify where MultiMarkdown should look for the files to be included.  All files must be in this folder.  If this folder is not specified, then MultiMarkdown will look in the same folder as the parent file.

{{links.txt}}
