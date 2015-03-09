cmake_minimum_required(VERSION 2.8)

# ====================
#	MultiMarkdown 
# ====================

project(MultiMarkdown)

set (MMD_VERSION_MAJOR 4)
set (MMD_VERSION_MINOR 7)
set (MMD_VERSION_PATCH 1)

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
	src/text.c
	src/toc.c
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
	src/text.h
	src/toc.h
	src/transclude.h
	src/writer.h
)

set(scripts
	scripts/mmd
	scripts/mmd2all
	scripts/mmd2opml
	scripts/mmd2rtf
	scripts/mmd2odf
	scripts/mmd2pdf
	scripts/mmd2tex
	scripts/markdown
)



# TODO: build greg if not present
# Need to build parser.c via greg
add_custom_command (
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser.c
	COMMAND ${PROJECT_SOURCE_DIR}/greg/greg -o ${CMAKE_CURRENT_BINARY_DIR}/parser.c ${PROJECT_SOURCE_DIR}/src/parser.leg
)

include_directories( ${PROJECT_SOURCE_DIR}/src )

if (WIN32)

# We need strtok on Windows
	list (APPEND src_files src/strtok.c)
	list (APPEND header_files src/strtok.h)

# Use bat files instead of shell scripts
	set(scripts
		scripts/mmd.bat
		scripts/mmd2odf.bat
		scripts/mmd2opml.bat
		scripts/mmd2rtf.bat
		scripts/mmd2tex.bat
		scripts/markdown.bat
	)

	set (CPACK_GENERATOR NSIS)

endif (WIN32)


if (APPLE)
# Support back to 10.4 on OS X (this was chosen somewhat arbitrarily, and older versions may work)
   SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.4" CACHE STRING "Deployment target for OSX" FORCE)

# Compile for both processor types in Release version (ppc is no longer supported)
	if(CMAKE_BUILD_TYPE MATCHES "Release")
		SET (CMAKE_OSX_ARCHITECTURES x86_64;i386)
	endif(CMAKE_BUILD_TYPE MATCHES "Release")

	set (CPACK_GENERATOR PackageMaker)

endif (APPLE)


# ================
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


# =============
# multimarkdown
# =============

add_executable(multimarkdown
	src/multimarkdown.c
)

target_link_libraries (multimarkdown libMultiMarkdown)


# =========
# Installer
# =========

# (NOTE: README.html has to be generated before it can be used here)

install (TARGETS multimarkdown
	DESTINATION bin
	COMPONENT application
)

set (CPACK_COMPONENT_APPLICATION_DISPLAY_NAME "MultiMarkdown")
set (CPACK_COMPONENT_APPLICATION_DESCRIPTION 
   "Install the actual `multimarkdown` program.")


install (FILES ${scripts}
	DESTINATION bin
	COMPONENT scripts
	PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install (FILES ${PROJECT_SOURCE_DIR}/LICENSE.txt ${CMAKE_CURRENT_BINARY_DIR}/README.html
	DESTINATION notes
)

set (CPACK_COMPONENT_SCRIPTS_DISPLAY_NAME "Convenience scripts")
set (CPACK_COMPONENT_SCRIPTS_DESCRIPTION 
   "Install convenience scripts for common MultiMarkdown shortcuts, e.g. `mmd`, `mmd2tex`, etc.")

include (InstallRequiredSystemLibraries)

set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "MultiMarkdown - lightweight markup processor")
set (CPACK_PACKAGE_VENDOR "Fletcher T. Penney")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_BINARY_DIR}/README.html")
set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_BINARY_DIR}/README.html")
set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")
# set (CPACK_COMPONENTS_IGNORE_GROUPS 1)

set (CPACK_PACKAGE_VERSION "${MMD_VERSION_MAJOR}.${MMD_VERSION_MINOR}.${MMD_VERSION_PATCH}")
set (CPACK_PACKAGE_VERSION_MAJOR "${MMD_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${MMD_VERSION_MINOR}")

set (CPACK_COMPONENTS_ALL application scripts)

if (APPLE)
	set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-Mac-${CPACK_PACKAGE_VERSION}")
else (APPLE)
	set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CMAKE_SYSTEM_NAME}-${CPACK_PACKAGE_VERSION}")
endif (APPLE)


set (CPACK_COMPONENT_APPLICATION_GROUP "MultiMarkdown")
set (CPACK_COMPONENT_SCRIPTS_GROUP "MultiMarkdown")

set (CPACK_COMPONENT_GROUP_MULTIMARKDOWN_DESCRIPTION
   "The basic MultiMarkdown utility and convenience scripts.")

set (CPACK_SET_DESTDIR true)

if (WIN32)
	set (CPACK_INSTALL_PREFIX "")
	set (CPACK_NSIS_MODIFY_PATH ON)
else (WIN32)
	set (CPACK_INSTALL_PREFIX /usr/local)
endif (WIN32)


set (CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT})


include (CPack)


# ==========
# Test Suite
# ==========

enable_testing()

add_test( markdown-should-fail ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --Tidy --Flags="--compatibility" --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/Test)
add_test( markdown ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --Tidy --Flags="--compatibility" --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/Tests)

add_test ( mmd  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests)

add_test ( latex  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ latex --ext=tex)
add_test ( beamer  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/BeamerTests --Flags=-t\ beamer --ext=tex)
add_test ( memoir  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MemoirTests --Flags=-t\ memoir --ext=tex)

add_test ( lyx  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ lyx --ext=lyx)
add_test ( lyx-beamer  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/BeamerTests --Flags=-t\ lyx --ext=lyx)

add_test ( opml  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ opml --ext=opml)

add_test ( odf  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --Flags=-t\ odf --ext=fodt)

add_test ( critic-accept  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/CriticMarkup --Flags=-a --ext=htmla)
add_test ( critic-reject  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/CriticMarkup --Flags=-r --ext=htmlr)
add_test ( critic-highlight  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=${CMAKE_CURRENT_BINARY_DIR}/multimarkdown --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/CriticMarkup --Flags=-r\ -a --ext=htmlh)

add_test ( xslt-1  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=/bin/cat --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MultiMarkdownTests --TrailFlags=|\ ${PROJECT_SOURCE_DIR}/Support/bin/mmd2tex-xslt --ext=tex)
add_test ( xslt-2  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=/bin/cat --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/BeamerTests --TrailFlags=|\ ${PROJECT_SOURCE_DIR}/Support/bin/mmd2tex-xslt --ext=tex)
add_test ( xslt-3  ${PROJECT_SOURCE_DIR}/MarkdownTest/MarkdownTest.pl --Script=/bin/cat --testdir=${PROJECT_SOURCE_DIR}/MarkdownTest/MemoirTests --TrailFlags=|\ ${PROJECT_SOURCE_DIR}/Support/bin/mmd2tex-xslt --ext=tex)
