# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/aarch64/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/aarch64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/progetto_SO.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/progetto_SO.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/progetto_SO.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/progetto_SO.dir/flags.make

CMakeFiles/progetto_SO.dir/main.c.o: CMakeFiles/progetto_SO.dir/flags.make
CMakeFiles/progetto_SO.dir/main.c.o: /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/main.c
CMakeFiles/progetto_SO.dir/main.c.o: CMakeFiles/progetto_SO.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/progetto_SO.dir/main.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/progetto_SO.dir/main.c.o -MF CMakeFiles/progetto_SO.dir/main.c.o.d -o CMakeFiles/progetto_SO.dir/main.c.o -c /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/main.c

CMakeFiles/progetto_SO.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/progetto_SO.dir/main.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/main.c > CMakeFiles/progetto_SO.dir/main.c.i

CMakeFiles/progetto_SO.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/progetto_SO.dir/main.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/main.c -o CMakeFiles/progetto_SO.dir/main.c.s

# Object files for target progetto_SO
progetto_SO_OBJECTS = \
"CMakeFiles/progetto_SO.dir/main.c.o"

# External object files for target progetto_SO
progetto_SO_EXTERNAL_OBJECTS =

progetto_SO: CMakeFiles/progetto_SO.dir/main.c.o
progetto_SO: CMakeFiles/progetto_SO.dir/build.make
progetto_SO: CMakeFiles/progetto_SO.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable progetto_SO"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/progetto_SO.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/progetto_SO.dir/build: progetto_SO
.PHONY : CMakeFiles/progetto_SO.dir/build

CMakeFiles/progetto_SO.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/progetto_SO.dir/cmake_clean.cmake
.PHONY : CMakeFiles/progetto_SO.dir/clean

CMakeFiles/progetto_SO.dir/depend:
	cd /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/cmake-build-debug /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/cmake-build-debug /Users/tommy/Desktop/Tommy/sistemi-operativi-progetto/cmake-build-debug/CMakeFiles/progetto_SO.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/progetto_SO.dir/depend

