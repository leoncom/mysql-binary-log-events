# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs

# Include any dependencies generated for this target.
include examples/CMakeFiles/basic-1.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/basic-1.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/basic-1.dir/flags.make

examples/CMakeFiles/basic-1.dir/basic-1.cpp.o: examples/CMakeFiles/basic-1.dir/flags.make
examples/CMakeFiles/basic-1.dir/basic-1.cpp.o: examples/basic-1.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/basic-1.dir/basic-1.cpp.o"
	cd /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/basic-1.dir/basic-1.cpp.o -c /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples/basic-1.cpp

examples/CMakeFiles/basic-1.dir/basic-1.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/basic-1.dir/basic-1.cpp.i"
	cd /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples/basic-1.cpp > CMakeFiles/basic-1.dir/basic-1.cpp.i

examples/CMakeFiles/basic-1.dir/basic-1.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/basic-1.dir/basic-1.cpp.s"
	cd /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples/basic-1.cpp -o CMakeFiles/basic-1.dir/basic-1.cpp.s

examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.requires:

.PHONY : examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.requires

examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.provides: examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.requires
	$(MAKE) -f examples/CMakeFiles/basic-1.dir/build.make examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.provides.build
.PHONY : examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.provides

examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.provides.build: examples/CMakeFiles/basic-1.dir/basic-1.cpp.o


# Object files for target basic-1
basic__1_OBJECTS = \
"CMakeFiles/basic-1.dir/basic-1.cpp.o"

# External object files for target basic-1
basic__1_EXTERNAL_OBJECTS =

examples/basic-1: examples/CMakeFiles/basic-1.dir/basic-1.cpp.o
examples/basic-1: examples/CMakeFiles/basic-1.dir/build.make
examples/basic-1: lib/libmysqlstream.a
examples/basic-1: libbinlogevents/lib/libbinlogevents.a
examples/basic-1: /usr/lib/libz.dylib
examples/basic-1: /usr/local/mysql/lib/libmysqlclient.a
examples/basic-1: examples/CMakeFiles/basic-1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable basic-1"
	cd /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/basic-1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/basic-1.dir/build: examples/basic-1

.PHONY : examples/CMakeFiles/basic-1.dir/build

examples/CMakeFiles/basic-1.dir/requires: examples/CMakeFiles/basic-1.dir/basic-1.cpp.o.requires

.PHONY : examples/CMakeFiles/basic-1.dir/requires

examples/CMakeFiles/basic-1.dir/clean:
	cd /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples && $(CMAKE_COMMAND) -P CMakeFiles/basic-1.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/basic-1.dir/clean

examples/CMakeFiles/basic-1.dir/depend:
	cd /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples /Users/daniel/Downloads/mysql-binary-log-events-1.0.2-labs/examples/CMakeFiles/basic-1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/basic-1.dir/depend

