##*****************************************************************************
##
## Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
## Contact: http://www.dewtronics.com/
##
## This file is part of the KJVCanOpener Application as originally written
## and developed for Bethel Church, Festus, MO.
##
## GNU General Public License Usage
## This file may be used under the terms of the GNU General Public License
## version 3.0 as published by the Free Software Foundation and appearing
## in the file gpl-3.0.txt included in the packaging of this file. Please
## review the following information to ensure the GNU General Public License
## version 3.0 requirements will be met:
## http://www.gnu.org/copyleft/gpl.html.
##
## Other Usage
## Alternatively, this file may be used in accordance with the terms and
## conditions contained in a signed written agreement between you and
## Dewtronics.
##
##*****************************************************************************

# Note: 3.21 needed for PROJECT_IS_TOP_LEVEL
cmake_minimum_required(VERSION 3.21...3.28)

include(cmake/GenerateVersion.cmake)

set(LANGUAGES C CXX)
if(WIN32)
	list(APPEND LANGUAGES RC)
endif()

set(CMAKE_PROJECT_VERSION_PRERELEASE "AlphaTest" CACHE STRING "PreRelease Descriptor")		# Set to empty string for official release

# Find Git executable
find_program(GIT_EXECUTABLE git)
if(NOT GIT_EXECUTABLE)
	message(FATAL_ERROR "Git executable not found.")
endif()

# Custom command to get git describe output
set(GIT_VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/git_version.txt)
set(GIT_VERSION_CHECK_FILE ${CMAKE_CURRENT_BINARY_DIR}/git_version_check.txt)
add_custom_command(
	OUTPUT ${GIT_VERSION_CHECK_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo \#undef Q > ${GIT_VERSION_CHECK_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo \#undef QUOTE >> ${GIT_VERSION_CHECK_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo \#define Q\(text\) \#text >> ${GIT_VERSION_CHECK_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo \#define QUOTE\(text\) Q\(text\) >> ${GIT_VERSION_CHECK_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo_append \#define GIT_VERSION\  >> ${GIT_VERSION_CHECK_FILE}
	COMMAND ${GIT_EXECUTABLE} describe --always --dirty=-dirty >> ${GIT_VERSION_CHECK_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo \#define GIT_VERSION_STR QUOTE\(GIT_VERSION\) >> ${GIT_VERSION_CHECK_FILE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
)

# Custom target that depends on the custom command
add_custom_target(git_version_target
	DEPENDS ${GIT_VERSION_CHECK_FILE}
	BYPRODUCTS ${GIT_VERSION_FILE}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GIT_VERSION_CHECK_FILE} ${GIT_VERSION_FILE}
	COMMAND ${CMAKE_COMMAND} -E rm -f ${GIT_VERSION_CHECK_FILE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
)

project(KJPBS VERSION 4.0.0 DESCRIPTION "KJPBS" LANGUAGES ${LANGUAGES})
project_genver(HEADER_FILE
	DEFINES
		LEGAL_COPYRIGHT="Copyright\(c\)2012-2025 Donna Whisnant, a.k.a. Dewtronics"
		ORGNAME="Dewtronics"
		ORGDOMAIN="dewtronics.com"
)

add_subdirectory(KJVCanOpener)
add_subdirectory(KJVDataParse)
add_subdirectory(KJVDataDump)
add_subdirectory(KJVDictWord)
add_subdirectory(KJVDiff)
add_subdirectory(KJVLookup)
add_subdirectory(KJVPhraseDump)
add_subdirectory(KJVPhraseSearch)
add_subdirectory(KJVSearch)
add_subdirectory(KJVSumThing)
# -------------------------------------
add_subdirectory(MyBibleParse)
# -------------------------------------
add_subdirectory(eSwordParse)
add_subdirectory(eSwordDecrypt2)
# -------------------------------------
add_subdirectory(ELSSearch)
# -------------------------------------
add_subdirectory(CrossRefParse)
