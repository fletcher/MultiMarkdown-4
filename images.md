latex input:	mmd-tufte-handout-header
Title:	Images
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-03-02 
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Images ##

The basic syntax for images in Markdown is:


	![Alt text](/path/to/img.jpg)

	![Alt text](/path/to/img.jpg "Optional title")


	![Alt text][id]

	[id]: url/to/image  "Optional title attribute"


In addition to the attributes you can use with links and images (described in the previous section), MultiMarkdown also adds a few additional things.  If an image is the only thing in a paragraph, it is treated as a block level element:

	This image (![Alt text](/path/to/img.jpg))
	is different than the following image:

	![Alt text](/path/to/img.jpg)

The resulting HTML is:

	<p>This image (<img src="/path/to/img.jpg" alt="Alt text" />)
	is different than the following image:</p>
	
	<figure>
	<img src="/path/to/img.jpg" alt="Alt text" />
	<figcaption>Alt text</figcaption>
	</figure>

The first one would be an inline image.  The second one (in HTML) would be wrapped in an HTML `figure` element.  In this case, the `alt` text is also used as a figure caption, and can contain MultiMarkdown syntax (e.g. bold, emph, etc.).  The alt text is not specifically designed to limit which MultiMarkdown is supported, but there will be limits and block level elements aren't supported.
