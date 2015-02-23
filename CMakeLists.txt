cmake_minimum_required(VERSION 3.0)

project(MultiMarkdown)

set(src_files
	src/GLibFacade.c
	src/beamer.c
	src/critic.c
	src/html.c
	src/latex.c
	src/lyx.c
	src/lyxbeamer.c
	src/memoir.c
	src/multimarkdown.c
	src/odf.c
	src/opml.c
	src/parse_utilities.c
	src/rng.c
	src/rtf.c
	src/strtok.c
	src/text.c
	src/transclude.c
	src/writer.c
)

set(header_files
	src/GLibFacade.h
	src/beamer.h
	src/critic.h
	src/glib.h
	src/html.h
	src/latex.h
	src/libMultiMarkdown.h
	src/lyx.h
	src/lyxbeamer.h
	src/memoir.h
	src/odf.h
	src/opml.h
	src/parser.h
	src/rtf.h
	src/strtok.h
	src/text.h
	src/transclude.h
	src/writer.h
)

# TODO: build greg if not present
# Need to build parser.c via greg
add_custom_command (
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser.c
	COMMAND ${PROJECT_SOURCE_DIR}/greg/greg -o ${CMAKE_CURRENT_BINARY_DIR}/parser.c ${PROJECT_SOURCE_DIR}/src/parser.leg
)

include_directories( ${PROJECT_SOURCE_DIR}/src )


# libMultiMarkdown
# ================
# Since "MultiMarkdown" and "multimarkdown" are almost the same, 
# it causes errors on case insensitive file systems.
# add "lib" and remove the extra "liblibFOO" later

add_library(libMultiMarkdown STATIC
	${src_files}
	${header_files}
	${CMAKE_CURRENT_BINARY_DIR}/parser.c
)

# remove the extra from "liblibFOO"
SET_TARGET_PROPERTIES(libMultiMarkdown PROPERTIES PREFIX "")


# multimarkdown
# =============

add_executable(multimarkdown
	src/multimarkdown.c
)

target_link_libraries (multimarkdown libMultiMarkdown)


# Test Suite
# TODO: This isn't useful yet

enable_testing()

add_test( markdown ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=../build/multimarkdown --Tidy --Flags="--compatibility")

add_test ( mmd  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=../build/multimarkdown --testdir=MarkdownTest/MultiMarkdownTests)

