all:
	mkdir -p build
	cd build && cmake .. && make
	cp build/pixelFEM .
