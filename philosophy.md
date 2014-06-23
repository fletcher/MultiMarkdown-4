latex input:	mmd-tufte-handout-header
Title:	The Philosophy Behind MultiMarkdown  
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2014-06-23  
Base Header Level:	3  
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


# The Philosophy Behind MultiMarkdown #

My vision for MultiMarkdown was inspired by my understanding of what made Markdown so wonderful.  Markdown is simple.  It's easy to remember.  It's intuitive to read.  Markdown avoids the "everything but the kitchen sink" problem.

My goal for MultiMarkdown is that it should be useable for 80% of the documents that 80% of people write.  Obviously that is not a precise estimate, but the idea is that *most* people can write *most* of their documents using it.  Some people can write everything in MMD.  Some people can write very little in MMD.  MultiMarkdown (by itself) would not be very good for writing a comic book, for example.  It's perfect for writing a novel.

A central tenet of MultiMarkdown is that the focus is on *content*, not *presentation*.  I  honestly couldn't care whether you want to use Arial, Helvetica, or Comic Sans for your masterpiece.  The presentation/styling/appearance is for you to decide.  You pick the fonts.  You pick the colors.  What I care about with MultiMarkdown is that most (not necessarily all) of the *meaning* of the document is represented --- this is a list, that is a table, this is a top-level heading, etc.

A well written MultiMarkdown document will look reasonably good whether you output to HTML, LaTeX, OpenDocument, etc.  It might not look perfect.  A page might break at an inopportune place.  The title page of a LaTeX document doesn't have an exact analogy in HTML.  HTML doesn't handle page breaks well.  

If you're writing your thesis, publishing a book, or submitting a document to the board of directors --- by all means write in MultiMarkdown.  Focus on the content and overall structure.  And when you're ready, convert to your desired output format.  Proofread.  And when you're sure that you like what you've got, *then* focus on the aesthetics.  Insert a page break.  Tweak fonts.  Go wild.  But do it in a tool appropriate for the format you're using.  This might be a good programmer's text editor for HTML and CSS.  It might be [LyX]. It might be [LibreOffice].

## The Purpose of MultiMarkdown ##

In the years since MultiMarkdown was first released, I've received countless emails of all kinds.  A group stands out that seems to point to a philosophical difference between types of users.

Computers are wonderful for doing the tedious sorts of things that humans tend to not enjoy and to suck at.  For example, I don't want to have to add all the columns in a spreadsheet by hand.  That's what computers are for.  Conversely, I don't want to read a novel written by a computer (at least not yet...)

As applied to MultiMarkdown, it's purpose is to handle the tedium of applying repetitive formatting rules to text.  For example, having to wrap every single paragraph in `<p>` tags for a web site is really tedious.  The computer should be able to handle that easily.

But the user should still understand *why* those `<p>` tags are necessary.  The goal of MultiMarkdown is not to say, "Don't worry your pretty little head about complicated things like HTML or LaTeX."  The goal is to allow you to learn and appreciate things like HTML and LaTeX without most of the tedium that goes along with marking up a document by hand.

MultiMarkdown is not a magical "black box" that converts plain text to HTML with a lot of hand-waving and "pay no attention to the man behind the curtain."  Programs that use this approach tend to result in crappy output (e.g. Microsoft products, most apps to create web pages "for you", etc.)

So when you're trying to do something fancy, or trying to trouble-shoot a problem, start at the end.  Look at the HTML/LaTeX/whatever that is generated and see what's going on at a fundamental level.  Once you understand that, then look at what MultiMarkdown is doing.  I believe you'll have an easier time solving problems, and probably learn a thing or two along the way....


## Feature Requests ##

I often get feature requests.  Some requests are really good ideas and I implement them.  Some are really good ideas and I don't implement them.  Some, however, miss the point of MultiMarkdown entirely.

I completely understand that somewhere out there, somebody's life would be complete if MultiMarkdown had a feature that drew a picture of a bunny after every 15th word of a MultiMarkdown document.  But that feature would be absolutely useless to everyone else on the planet.  Add enough of those sorts of features, and you end up with Microsoft Word.  Which I am sure is the only application that some people are able to use, *precisely because* it draws bunnies, and even lets you choose which color and breed of bunny to use.  And whether the bunny is left- or right-pawed.

I am not going to program MultiMarkdown to draw bunnies.

Instead, if there is something that you wish MultiMarkdown would do, consider the following:

1. First, make sure the feature you want doesn't already exist.  Read the documentation.  Look at the [Sample Gallery].

2. If it's not there, consider whether you can "hijack" an existing feature.  For example, I needed to create a PDF to print a book of poetry.  MMD didn't have a "poetry" feature.  But it did have code blocks, which are essentially the same thing, except poetry doesn't usually use monospaced fonts.  Voila, I used code blocks for all of the poems, and then changed the LaTeX output to refrain from using monospaced fonts when displaying code.

3. Still stuck? --- ask for help.  The [discussion list] is a great place to get help, as is the [support site].

4. But before requesting a new feature, honestly ask yourself how many other people need it.  Ask yourself why, if it's such a great idea, it hasn't been implemented yet. 

5. If your idea isn't really useful to other people, then that makes it the *perfect* opportunity to develop some new skills.  Grab a copy of the MultiMarkdown source, and start hacking away to add what you want.  Test it out.  If it's really great, share it on the discussion list to let others use it.  Convince everyone that it simply *must* be added to the core source.
