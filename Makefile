fmt:
	clang-format -i include/capybara/*.h include/capybara/*/*.h tests/*.cpp

test: build
	cd build && make -p tests
	#build/tests/tests

build: CMakeLists.txt
	mkdir -p build
	cd build && cmake ..

all: build

clean:
	rm -rf build

.PHONY: fmt test clean
