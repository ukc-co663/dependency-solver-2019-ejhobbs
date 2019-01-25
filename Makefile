.phony all: compile

.phony dependencies:
	apt-get install -y cmake ninja

cmake: dependencies
	cmake . -GNinja

compile: cmake
	ninja
