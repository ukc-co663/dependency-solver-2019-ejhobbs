# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ejhobbs/git/dependency-solver-2019-ejhobbs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ejhobbs/git/dependency-solver-2019-ejhobbs

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ejhobbs/git/dependency-solver-2019-ejhobbs/CMakeFiles /home/ejhobbs/git/dependency-solver-2019-ejhobbs/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ejhobbs/git/dependency-solver-2019-ejhobbs/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named solve

# Build rule for target.
solve: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 solve
.PHONY : solve

# fast build rule for target.
solve/fast:
	$(MAKE) -f CMakeFiles/solve.dir/build.make CMakeFiles/solve.dir/build
.PHONY : solve/fast

#=============================================================================
# Target rules for targets named cJSON

# Build rule for target.
cJSON: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 cJSON
.PHONY : cJSON

# fast build rule for target.
cJSON/fast:
	$(MAKE) -f CMakeFiles/cJSON.dir/build.make CMakeFiles/cJSON.dir/build
.PHONY : cJSON/fast

lib/cJSON/cJSON.o: lib/cJSON/cJSON.c.o

.PHONY : lib/cJSON/cJSON.o

# target to build an object file
lib/cJSON/cJSON.c.o:
	$(MAKE) -f CMakeFiles/cJSON.dir/build.make CMakeFiles/cJSON.dir/lib/cJSON/cJSON.c.o
.PHONY : lib/cJSON/cJSON.c.o

lib/cJSON/cJSON.i: lib/cJSON/cJSON.c.i

.PHONY : lib/cJSON/cJSON.i

# target to preprocess a source file
lib/cJSON/cJSON.c.i:
	$(MAKE) -f CMakeFiles/cJSON.dir/build.make CMakeFiles/cJSON.dir/lib/cJSON/cJSON.c.i
.PHONY : lib/cJSON/cJSON.c.i

lib/cJSON/cJSON.s: lib/cJSON/cJSON.c.s

.PHONY : lib/cJSON/cJSON.s

# target to generate assembly for a file
lib/cJSON/cJSON.c.s:
	$(MAKE) -f CMakeFiles/cJSON.dir/build.make CMakeFiles/cJSON.dir/lib/cJSON/cJSON.c.s
.PHONY : lib/cJSON/cJSON.c.s

main.o: main.c.o

.PHONY : main.o

# target to build an object file
main.c.o:
	$(MAKE) -f CMakeFiles/solve.dir/build.make CMakeFiles/solve.dir/main.c.o
.PHONY : main.c.o

main.i: main.c.i

.PHONY : main.i

# target to preprocess a source file
main.c.i:
	$(MAKE) -f CMakeFiles/solve.dir/build.make CMakeFiles/solve.dir/main.c.i
.PHONY : main.c.i

main.s: main.c.s

.PHONY : main.s

# target to generate assembly for a file
main.c.s:
	$(MAKE) -f CMakeFiles/solve.dir/build.make CMakeFiles/solve.dir/main.c.s
.PHONY : main.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... solve"
	@echo "... cJSON"
	@echo "... lib/cJSON/cJSON.o"
	@echo "... lib/cJSON/cJSON.i"
	@echo "... lib/cJSON/cJSON.s"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

