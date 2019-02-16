.PHONY: all
all: compile

dependencies:
	apt-get update
	apt-get install -y cmake ninja-build clang check

cmake: dependencies
	cmake -GNinja .

compile: cmake
	ninja
