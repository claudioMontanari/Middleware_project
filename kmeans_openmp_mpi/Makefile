CC := gcc
LD := gcc

GCC := gcc
GPP := g++

MPICC := mpicc

ROOT ?= ./

MAKE := make

CFLAGS += -g -O3 -mavx2 -Wall $(LDLIB) -Winline -fopenmp

LDFLAGS += -lm -fopenmp

.PHONY: all clean

BINS = kmeans kmeans_no_omp test

all: $(BINS)

kmeans: benchmark_mpi.c benchmark_mpi.h
	$(MPICC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -DUSE_OMP

kmeans_no_omp: benchmark_mpi.c benchmark_mpi.h
	$(MPICC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

report.pdf: report.tex
	pdflatex report.tex
	pdflatex report.tex

clean:
	rm -f $(BINS) *.o *.so
