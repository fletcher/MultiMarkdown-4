latex input:	mmd-tufte-handout-header
Title:	Definition Lists
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-01-04 
Base Header Level:	2
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


## Definition Lists ##

MultiMarkdown has support for definition lists using the same syntax used in
[PHP Markdown Extra][]. Specifically:

	Apple
	:	Pomaceous fruit of plants of the genus Malus in 
		the family Rosaceae.
	:	An american computer company.
	
	Orange
	:	The fruit of an evergreen tree of the genus Citrus.


becomes:

> Apple
> : Pomaceous fruit of plants of the genus Malus in 
>		the family Rosaceae.
> : An american computer company.
>
> Orange
> : The fruit of an evergreen tree of the genus Citrus.

You can have more than one term per definition by placing each term on a
separate line. Each definition starts with a colon, and you can have more than
one definition per term. You may optionally have a blank line between the last
term and the first definition.

Definitions may contain other block level elements, such as lists,
blockquotes, or other definition lists.

Unlike PHP Markdown Extra, all definitions are wrapped in `<p>` tags. First, I
was unable to get Markdown *not* to create paragraphs. Second, I didn't see
where it mattered - the only difference seems to be aesthetic, and I actually
prefer the `<p>` tags in place. Let me know if this is a problem.

See the [PHP Markdown Extra][] page for more information.
