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


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_SOURCE_DIR = /home/jzx/StudyCpp/sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jzx/StudyCpp/sylar

# Include any dependencies generated for this target.
include CMakeFiles/test_address.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_address.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_address.dir/flags.make

CMakeFiles/test_address.dir/tests/test_address.cpp.o: CMakeFiles/test_address.dir/flags.make
CMakeFiles/test_address.dir/tests/test_address.cpp.o: tests/test_address.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jzx/StudyCpp/sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_address.dir/tests/test_address.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) -D__FILE__=\"tests/test_address.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_address.dir/tests/test_address.cpp.o -c /home/jzx/StudyCpp/sylar/tests/test_address.cpp

CMakeFiles/test_address.dir/tests/test_address.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_address.dir/tests/test_address.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_address.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jzx/StudyCpp/sylar/tests/test_address.cpp > CMakeFiles/test_address.dir/tests/test_address.cpp.i

CMakeFiles/test_address.dir/tests/test_address.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_address.dir/tests/test_address.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_address.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jzx/StudyCpp/sylar/tests/test_address.cpp -o CMakeFiles/test_address.dir/tests/test_address.cpp.s

# Object files for target test_address
test_address_OBJECTS = \
"CMakeFiles/test_address.dir/tests/test_address.cpp.o"

# External object files for target test_address
test_address_EXTERNAL_OBJECTS =

bin/test_address: CMakeFiles/test_address.dir/tests/test_address.cpp.o
bin/test_address: CMakeFiles/test_address.dir/build.make
bin/test_address: lib/libsylar.so
bin/test_address: /usr/local/lib/libyaml-cpp.a
bin/test_address: CMakeFiles/test_address.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jzx/StudyCpp/sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin/test_address"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_address.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_address.dir/build: bin/test_address

.PHONY : CMakeFiles/test_address.dir/build

CMakeFiles/test_address.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_address.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_address.dir/clean

CMakeFiles/test_address.dir/depend:
	cd /home/jzx/StudyCpp/sylar && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jzx/StudyCpp/sylar /home/jzx/StudyCpp/sylar /home/jzx/StudyCpp/sylar /home/jzx/StudyCpp/sylar /home/jzx/StudyCpp/sylar/CMakeFiles/test_address.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_address.dir/depend

