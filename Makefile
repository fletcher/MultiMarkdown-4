# This makefile works on Mac OS, Ubuntu, Fedora, and presumably other *.nix'es
# Feel free to submit pull requests if you have a proposed change.
# Please explain your change so I'll understand what you need, and why - 
# I don't claim to be an expert at this aspect! 
#
#
# You can use `make pkg-install` and `make pkg-install-scripts` to install to 
# a custom directory (useful when building package installers):
#
#		make pkg-install DESTDIR=/some/folder/to/be/created
#
# - Fletcher T. Penney

CFLAGS ?= -Wall -g -O3 -include GLibFacade.h
PROGRAM = multimarkdown
VERSION = 4.7

OBJS= multimarkdown.o parse_utilities.o parser.o GLibFacade.o writer.o text.o html.o latex.o memoir.o beamer.o lyx.o lyxbeamer.o opml.o odf.o critic.o rng.o rtf.o transclude.o toc.o

# Common prefix for installation directories.
# NOTE: This directory must exist when you start the install.
prefix = /usr/local
exec_prefix = $(prefix)
# Where to put the executable
bindir = $(exec_prefix)/bin

# Allow for linking any libraries statically
# This only matters on some OS/environments, and 
# only if you move the binary to a different OS/environment
# `make static` may not work on all OS versions (e.g. Mac OS X)
ifeq ($(MAKECMDGOALS),static)
LDFLAGS += -static -static-libgcc
endif

# OUR_GREG: the version of greg in a submodule
# GREG: the path to greg we want to use for parser.leg
#
# This way we can pass GREG=/usr/local/bin/greg in on
# the command line if we have greg installed already.

OUR_GREG=greg/greg
GREG?=$(OUR_GREG)

ALL : $(PROGRAM) enumMap.txt
static : $(PROGRAM) enumMap.txt

%.o : %.c parser.h
	$(CC) -c $(CFLAGS) -o $@ $<

parser.c : parser.leg $(GREG) parser.h
	$(GREG) -o parser.c parser.leg

$(OUR_GREG): greg
	$(MAKE) -C greg

$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

install: $(PROGRAM) | $(prefix)/bin
	install -m 0755 multimarkdown $(prefix)/bin

$(prefix)/bin:
	-mkdir $(prefix)/bin  2>/dev/null

install-scripts:
	install -m 0755 scripts/* $(prefix)/bin

pkg-install: $(PROGRAM) | $(DESTDIR)$(prefix)/bin
	install -m 0755 multimarkdown $(DESTDIR)$(prefix)/bin

$(DESTDIR)$(prefix)/bin:
	-mkdir -p $(DESTDIR)$(prefix)/bin  2>/dev/null

pkg-install-scripts:
	install -m 0755 scripts/* $(DESTDIR)$(prefix)/bin

clean:
	rm -f $(PROGRAM) $(OBJS) parser.c enumMap.txt speed*.txt; \
	rm -rf mac_installer/Package_Root/usr/local/bin mac_installer/Support_Root mac_installer/*.pkg; \
	rm -f mac_installer/Resources/*.html; \
	rm -rf build

# Build for windows on a *nix machine with MinGW installed
windows: parser.c
	/usr/bin/i586-mingw32msvc-cc -c -Wall -O3 *.c
	/usr/bin/i586-mingw32msvc-cc *.o -Wl,--dy -o multimarkdown.exe

# Test program against MMD Test Suite
test: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --Tidy --Flags="--compatibility"; \
	echo ""; \
	echo "** It's expected that we fail the \"Ordered and unordered lists\" test **"; \
	echo ""; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --Tidy --Flags="--compatibility" --testdir=Test
	
test-mmd: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests

test-compat: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=CompatibilityTests --Flags="--compatibility"

test-latex: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests --Flags="-t latex" --ext="tex"

test-beamer: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=BeamerTests --Flags="-t beamer" --ext="tex"

test-memoir: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MemoirTests --Flags="-t memoir" --ext="tex"

test-lyx: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests --Flags="-t lyx" --ext="lyx"

test-lyx-beamer: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=BeamerTests --Flags="-t lyx" --ext="lyx"

test-opml: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests --Flags="-t opml" --ext="opml"

test-odf: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests --Flags="-t odf" --ext="fodt"

test-xslt: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=MultiMarkdownTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext="tex"; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=BeamerTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext="tex"; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=MemoirTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext="tex"; \

test-critic-accept: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=CriticMarkup --Flags="-a" --ext="htmla"

test-critic-reject: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=CriticMarkup --Flags="-r" --ext="htmlr"

test-critic-highlight: $(PROGRAM)
	-cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=CriticMarkup --Flags="-a -r" --ext="htmlh"

test-all: $(PROGRAM) test test-mmd test-compat test-latex test-beamer test-memoir test-opml test-odf test-critic-accept test-critic-reject test-critic-highlight test-lyx test-lyx-beamer

test-memory: $(PROGRAM)
	valgrind --leak-check=full ./$(PROGRAM) MarkdownTest/Tests/*.text MarkdownTest/MultiMarkdownTests/*.text > /dev/null

test-memory-latex: $(PROGRAM)
	valgrind --leak-check=full ./$(PROGRAM) -t latex MarkdownTest/Tests/*.text MarkdownTest/MultiMarkdownTests/*.text > /dev/null

test-memory-beamer: $(PROGRAM)
	valgrind --leak-check=full ./$(PROGRAM) -t beamer MarkdownTest/Tests/*.text MarkdownTest/MultiMarkdownTests/*.text > /dev/null

test-memory-memoir: $(PROGRAM)
	valgrind --leak-check=full ./$(PROGRAM) -t memoir MarkdownTest/Tests/*.text MarkdownTest/MultiMarkdownTests/*.text > /dev/null

test-memory-lyx: $(PROGRAM)
	valgrind --leak-check=full ./$(PROGRAM) -t lyx MarkdownTest/Tests/*.text MarkdownTest/MultiMarkdownTests/*.text MarkdownTest/BeamerTests/*.text > /dev/null

test-memory-odf: $(PROGRAM)
	valgrind --leak-check=full ./$(PROGRAM) -t odf MarkdownTest/Tests/*.text MarkdownTest/MultiMarkdownTests/*.text > /dev/null

test-memory-opml: $(PROGRAM)
	valgrind --leak-check=full ./$(PROGRAM) -t opml MarkdownTest/Tests/*.text MarkdownTest/MultiMarkdownTests/*.text > /dev/null

enumMap.txt: parser.h
	./enumsToPerl.pl libMultiMarkdown.h enumMap.txt

speed.txt: MarkdownTest/Tests/Markdown\ Documentation\ -\ Basics.text
	@ cp MarkdownTest/Tests/Markdown\ Documentation\ -\ Basics.text speed.txt

speed2.txt: speed.txt
	@ cat speed.txt speed.txt > speed2.txt

speed4.txt: speed2.txt
	@ cat speed2.txt speed2.txt > speed4.txt

speed8.txt: speed4.txt
	@ cat speed4.txt speed4.txt > speed8.txt

speed16.txt: speed8.txt
	@ cat speed8.txt speed8.txt > speed16.txt

speed32.txt: speed16.txt
	@ cat speed16.txt speed16.txt > speed32.txt

speed64.txt: speed32.txt
	 @ cat speed32.txt speed32.txt > speed64.txt

speed128.txt: speed64.txt
	@ cat speed64.txt speed64.txt > speed128.txt

speed256.txt: speed128.txt
	@ cat speed128.txt speed128.txt > speed256.txt

speed512.txt: speed256.txt
	@ cat speed256.txt speed256.txt > speed512.txt

# Compare regular with compatibility mode
test-speed: $(PROGRAM) speed512.txt
	time ./$(PROGRAM) speed512.txt > /dev/null
	time ./$(PROGRAM) -c speed512.txt > /dev/null

# Compare with peg-markdown (if installed)
test-speed-jgm: $(PROGRAM) speed512.txt
	time ./$(PROGRAM) speed512.txt > /dev/null
	time ./$(PROGRAM) -c speed512.txt > /dev/null
	time peg-markdown speed512.txt > /dev/null

# Compare with original Markdown.pl
# running tests on Markdown.pl with larger files will take a *long* time
test-speed-gruber: speed64.txt
	time ./$(PROGRAM) -c speed64.txt > /dev/null
	time MarkdownTest/Markdown.pl speed64.txt > /dev/null

# Build using Xcode (more compatible across legacy OS/Hardware)
xcode: 
	xcodebuild
	cp build/Release/multimarkdown .

# Build Mac Installer
mac-installer: xcode
	mkdir -p mac_installer/Package_Root/usr/local/bin
	mkdir -p mac_installer/Support_Root/Library/Application\ Support
	mkdir -p mac_installer/Resources
	rm -rf mac_installer/Support_Root
	cp build/Release/multimarkdown scripts/mmd* mac_installer/Package_Root/usr/local/bin/
	./multimarkdown README.md > mac_installer/Resources/README.html
	./multimarkdown mac_installer/Resources/Welcome.txt > mac_installer/Resources/Welcome.html
	./multimarkdown LICENSE > mac_installer/Resources/License.html
	./multimarkdown mac_installer/Resources/Support_Welcome.txt > mac_installer/Resources/Support_Welcome.html
	git clone Support mac_installer/Support_Root/Library/Application\ Support/MultiMarkdown
	cd mac_installer; /Applications/PackageMaker.app/Contents/MacOS/PackageMaker \
	--doc "Make Support Installer.pmdoc" \
	--title "MultiMarkdown Support Files" \
	--version $(VERSION) \
	--filter "\.DS_Store" \
	--filter "\.git" \
	--id net.fletcherpenney.MMD-Support.pkg \
	--domain user \
	--out "MultiMarkdown-Support-Mac-$(VERSION).pkg" \
	--no-relocate; \
	/Applications/PackageMaker.app/Contents/MacOS/PackageMaker \
	--doc "Make OS X Installer.pmdoc" \
	--title "MultiMarkdown" \
	--version $(VERSION) \
	--filter "\.DS_Store" \
	--filter "\.git" \
	--id net.fletcherpenney.multimarkdown.pkg \
	--out "MultiMarkdown-Mac-$(VERSION).pkg"
	cd mac_installer; zip -r MultiMarkdown-Mac-$(VERSION).zip MultiMarkdown-Mac-$(VERSION).pkg
	cd mac_installer; zip -r MultiMarkdown-Support-Mac-$(VERSION).zip MultiMarkdown-Support-Mac-$(VERSION).pkg	

# Prepare README and other files to create the BitRock installer
win-prep: 
	mkdir -p windows_installer
	cp multimarkdown.exe windows_installer/
	cp README.md windows_installer/README.txt
	./multimarkdown LICENSE > windows_installer/LICENSE.html

# After creating the installer with BitRock, package it up
win-installer:
	zip -r windows_installer/MultiMarkdown-Windows-$(VERSION).zip windows_installer/MMD-windows-$(VERSION).exe -x windows_installer/MultiMarkdown*.zip
	cd windows_installer; zip -r MultiMarkdown-Windows-Portable-$(VERSION).zip *.bat multimarkdown.exe README.txt LICENSE.html -x install_multimarkdown.bat

# Can make the portable version without BitRock 
win-portable:
	cd windows_installer; zip -r MultiMarkdown-Windows-Portable-$(VERSION).zip *.bat multimarkdown.exe README.txt LICENSE.html -x install_multimarkdown.bat
