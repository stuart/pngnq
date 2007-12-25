all: spatial_color_quant Makefile

clean:
	rm spatial_color_quant spatial_color_quant.o

spatial_color_quant: spatial_color_quant.o Makefile
	g++ -o spatial_color_quant spatial_color_quant.o

spatial_color_quant.o: spatial_color_quant.cpp Makefile
	g++ -Wall -pedantic -O3 -c spatial_color_quant.cpp -o spatial_color_quant.o
