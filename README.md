Title:	MultiMarkdown User's Guide  
Author:	Fletcher T. Penney  

These files are used to generate the MultiMarkdown User's Guide.  The document is, of course, written using MultiMarkdown.  It is also a *relatively* complex document, making use of several tricks with file transclusion among other things.  The idea is to repeat oneself as little as possible, and make it easier to keep all the formats and files in sync automatically.

Compiled versions of these documents can be found on the github site -- <http://fletcher.github.io/MultiMarkdown-4/>.

There are several commands you can use to compile the documentation:

* `make pdf` -- compile all of the files into a single PDF (using the Tufte Book class for LaTeX)

* `make html` -- compile each of the files into a separate HTML file, suitable for navigating on a web site.

* `make guide` -- make a single HTML file with all of the contents

* `make all` -- compile each file into the following document types: 

	* html 
	* LaTeX (which will then need to be compiled to PDF)
	* RTF
	* FODT

* `make odf` -- compile a single FODT document

