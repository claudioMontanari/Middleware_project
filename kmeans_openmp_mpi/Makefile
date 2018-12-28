CC := gcc
LD := gcc

GCC := gcc
GPP := g++

ROOT ?= ./

MAKE := make

CFLAGS += -g -O3 -mavx2 -Wall $(LDLIB) -Winline -fopenmp

LDFLAGS += -lm -lpthread 

.PHONY: all clean

BINS = benchmark test

all: $(BINS)

datapoint.o: datapoint.c 
	$(GCC) $(CFLAGS) -c -o $@ $<

kmeans.o: kmeans.c 
	$(GCC) $(CFLAGS) -c -o $@ $<

benchmark.o: benchmark.c
	$(GCC) $(CFLAGS) -c -o $@ $<

benchmark: benchmark.o kmeans.o datapoint.o
	$(GPP) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: test.c kmeans.o datapoint.o
	$(GPP) $(CFLAGS) -o $@ $^ $(LDFLAGS)

report.pdf: report.tex
	pdflatex report.tex
	pdflatex report.tex

clean:
	rm -f $(BINS) *.o *.so
