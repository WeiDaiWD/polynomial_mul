Compile = g++

all: measure

measure: *.cpp simd_poly.h
	$(Compile) *.cpp -O3 -mavx2 -o measure

clean:
	rm measure
