.PHONY: all
all: compile

dependencies:
	apt-get update
	apt-get install -y cmake ninja-build

cmake: dependencies
	cmake -GNinja .

compile: cmake
	ninja
