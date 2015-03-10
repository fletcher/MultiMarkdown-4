latex input:	mmd-tufte-handout-header
Title:	Installation
Author:	Fletcher T. Penney
Version:	{{version.txt}}
Revised:	2015-03-10  
Base Header Level:	3
CSS:	{{css.txt}}
MMD Footer:	links.txt
latex mode:	memoir
latex input:	mmd-tufte-handout-begin-doc
latex footer:	mmd-tufte-footer


# Installation #

You have several options for obtaining and installing MultiMarkdown:

* Binary installer -- available for:
	* Mac OS
	* Windows
* Use a third party package installer:
	* `homebrew` for Mac OS 
* Compile from source -- useful if you want to modify MMD's behavior, or if there isn't an available binary download for your operating system


## Mac OS ##


### Installer ###

You can download the installers from the MMD website [download] page.  You need the `Mac Installer`.  Download it.  Run it.  Done.

If you use older tools that were designed for MultiMarkdown version 3, you may need to use the `Mac Support Installer`.  This is also useful if you need the older XSLT based parsing tools.

If you plan on creating LaTeX documents, you should also download the [LaTeX Support Files] and install them into the appropriate location for your system and LaTeX software.

### Homebrew ###

You can use [homebrew](https://github.com/Homebrew/homebrew) to install:

	brew install multimarkdown

Or, if you want the latest updates between releases:

	brew install --HEAD multimarkdown

(**Note:** I use the `--HEAD` version on my own machine.)

### MacPorts ###

[MacPorts](https://www.macports.org/) has a package for MultiMarkdown:

	sudo port install multimarkdown

I don't maintain this package, and it will likely not point to the latest version. I don't recommend it.

## *nix ##

Unix/Linux users should use the instructions for compiling from source below.

## Windows ##

The easiest way to install MMD on Windows is the `MultiMarkdown-Windows`
installer from the [download] page and run it. The installer is built using
the `NSIS` package maker software via the `cmake` and `cpack` tools.

Just as with the Mac OS X version, the installer includes the `multimarkdown`
binary, as well as several convenience scripts.

You can use Windows Explorer to create shortcuts to the `multimarkdown`
binary, and adjust the properties to allow you to create "drag and drop"
versions of MMD as well.

You can also download a "Portable" version that can be run off USB thumb drives, for example.  It is also available on the [download] page.


## Free BSD ##

If you want to compile manually, you should be able to follow the directions for Linux below. However, apparently MultiMarkdown has been put in the ports tree, so you can also use:

	cd /usr/ports/textproc/multimarkdown
	make install

(I have not tested this myself, and cannot guarantee that it works properly.)


## Compile From Source ##


### Mac and *Nix Machines ###

*	Download the source from the [github] web site:

		git https://github.com/fletcher/MultiMarkdown-4.git

*	Update the submodules, including `greg`

		git submodule init
		git submodule update

*	 Compile:

		make

*	Run `make test-all | less`  (or `make test-all | grep failed` for a more concise version) to verify that the build is correct.  One of the tests is expected to fail ("Ordered and unordered lists"); the rest should pass on all systems.  
*	`make install` (as root) will install the software
*	`make install-scripts` will install the helper scripts for you (e.g.`mmd`, `mmd2tex`, etc.)
*	If you plan on creating LaTeX documents, you should also download the [LaTeX Support Files] and install them into the appropriate location for your system and LaTeX software.

MultiMarkdown includes a few other projects as submodules, but the only one you need to actually compile the code is the `greg` software.  Once compiled, MultiMarkdown has no external dependencies -- the binary is self-contained.  Therefore, it should basically compile and run anywhere. 


### Windows ###

Windows users can obtain the code in the same way, but will need to use their own compiler.  The way I compile for Windows is actually to use the `make windows` command running on a *nix system with MinGW installed.

The instructions for [peg-markdown] demonstrate how to compile a package for Windows.

Otherwise, you're on your own here.