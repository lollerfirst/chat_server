# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lollerfirst/dev/c/chat_server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lollerfirst/dev/c/chat_server/build

# Include any dependencies generated for this target.
include CMakeFiles/chatserver.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/chatserver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/chatserver.dir/flags.make

CMakeFiles/chatserver.dir/server.c.o: CMakeFiles/chatserver.dir/flags.make
CMakeFiles/chatserver.dir/server.c.o: ../server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lollerfirst/dev/c/chat_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/chatserver.dir/server.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/chatserver.dir/server.c.o -c /home/lollerfirst/dev/c/chat_server/server.c

CMakeFiles/chatserver.dir/server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/chatserver.dir/server.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/lollerfirst/dev/c/chat_server/server.c > CMakeFiles/chatserver.dir/server.c.i

CMakeFiles/chatserver.dir/server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/chatserver.dir/server.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/lollerfirst/dev/c/chat_server/server.c -o CMakeFiles/chatserver.dir/server.c.s

# Object files for target chatserver
chatserver_OBJECTS = \
"CMakeFiles/chatserver.dir/server.c.o"

# External object files for target chatserver
chatserver_EXTERNAL_OBJECTS =

chatserver: CMakeFiles/chatserver.dir/server.c.o
chatserver: CMakeFiles/chatserver.dir/build.make
chatserver: CMakeFiles/chatserver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lollerfirst/dev/c/chat_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable chatserver"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/chatserver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/chatserver.dir/build: chatserver

.PHONY : CMakeFiles/chatserver.dir/build

CMakeFiles/chatserver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/chatserver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/chatserver.dir/clean

CMakeFiles/chatserver.dir/depend:
	cd /home/lollerfirst/dev/c/chat_server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lollerfirst/dev/c/chat_server /home/lollerfirst/dev/c/chat_server /home/lollerfirst/dev/c/chat_server/build /home/lollerfirst/dev/c/chat_server/build /home/lollerfirst/dev/c/chat_server/build/CMakeFiles/chatserver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/chatserver.dir/depend
