cmake_minimum_required(VERSION 2.8)

project(MultiMarkdown)

set (MMD_VERSION_MAJOR 4)
set (MMD_VERSION_MINOR 7)

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


# Installer
# (NOTE: README.html has to be generated before it can be used here)

install (TARGETS multimarkdown DESTINATION bin)

include (InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MultiMarkdown - lightweight markup processor")
set(CPACK_PACKAGE_VENDOR "Fletcher T. Penney")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_BINARY_DIR}/README.html")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_BINARY_DIR}/README.html")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_COMPONENTS_IGNORE_GROUPS 1)

set (CPACK_PACKAGE_VERSION 4.7)
set (CPACK_PACKAGE_VERSION_MAJOR "${MMD_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${MMD_VERSION_MINOR}")

include (CPack)


# Test Suite
# TODO: This isn't useful yet

enable_testing()

add_test( markdown ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --Tidy --Flags="--compatibility" --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/Tests)

add_test ( mmd  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests)

add_test ( latex  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ latex --ext=tex)

add_test ( beamer  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/BeamerTests --Flags=-t\ beamer --ext=tex)

add_test ( memoir  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MemoirTests --Flags=-t\ memoir --ext=tex)

add_test ( lyx  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ lyx --ext=lyx)

add_test ( lyx-beamer  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/BeamerTests --Flags=-t\ lyx --ext=lyx)

add_test ( latex  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ latex --ext=tex)

add_test ( opml  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ opml --ext=opml)

add_test ( odf  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ odf --ext=fodt)
