# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/cache_schedule/leveldb

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/cache_schedule/leveldb/build

# Include any dependencies generated for this target.
include CMakeFiles/db_test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/db_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/db_test.dir/flags.make

CMakeFiles/db_test.dir/util/testutil.cc.o: CMakeFiles/db_test.dir/flags.make
CMakeFiles/db_test.dir/util/testutil.cc.o: ../util/testutil.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/cache_schedule/leveldb/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/db_test.dir/util/testutil.cc.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/db_test.dir/util/testutil.cc.o -c /home/ubuntu/cache_schedule/leveldb/util/testutil.cc

CMakeFiles/db_test.dir/util/testutil.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db_test.dir/util/testutil.cc.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/cache_schedule/leveldb/util/testutil.cc > CMakeFiles/db_test.dir/util/testutil.cc.i

CMakeFiles/db_test.dir/util/testutil.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db_test.dir/util/testutil.cc.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/cache_schedule/leveldb/util/testutil.cc -o CMakeFiles/db_test.dir/util/testutil.cc.s

CMakeFiles/db_test.dir/db/db_test.cc.o: CMakeFiles/db_test.dir/flags.make
CMakeFiles/db_test.dir/db/db_test.cc.o: ../db/db_test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/cache_schedule/leveldb/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/db_test.dir/db/db_test.cc.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/db_test.dir/db/db_test.cc.o -c /home/ubuntu/cache_schedule/leveldb/db/db_test.cc

CMakeFiles/db_test.dir/db/db_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db_test.dir/db/db_test.cc.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/cache_schedule/leveldb/db/db_test.cc > CMakeFiles/db_test.dir/db/db_test.cc.i

CMakeFiles/db_test.dir/db/db_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db_test.dir/db/db_test.cc.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/cache_schedule/leveldb/db/db_test.cc -o CMakeFiles/db_test.dir/db/db_test.cc.s

# Object files for target db_test
db_test_OBJECTS = \
"CMakeFiles/db_test.dir/util/testutil.cc.o" \
"CMakeFiles/db_test.dir/db/db_test.cc.o"

# External object files for target db_test
db_test_EXTERNAL_OBJECTS =

db_test: CMakeFiles/db_test.dir/util/testutil.cc.o
db_test: CMakeFiles/db_test.dir/db/db_test.cc.o
db_test: CMakeFiles/db_test.dir/build.make
db_test: libleveldb.a
db_test: lib/libgmockd.a
db_test: lib/libgtestd.a
db_test: third_party/benchmark/src/libbenchmark.a
db_test: /usr/lib/x86_64-linux-gnu/librt.so
db_test: CMakeFiles/db_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/cache_schedule/leveldb/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable db_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/db_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/db_test.dir/build: db_test

.PHONY : CMakeFiles/db_test.dir/build

CMakeFiles/db_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/db_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/db_test.dir/clean

CMakeFiles/db_test.dir/depend:
	cd /home/ubuntu/cache_schedule/leveldb/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/cache_schedule/leveldb /home/ubuntu/cache_schedule/leveldb /home/ubuntu/cache_schedule/leveldb/build /home/ubuntu/cache_schedule/leveldb/build /home/ubuntu/cache_schedule/leveldb/build/CMakeFiles/db_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/db_test.dir/depend

