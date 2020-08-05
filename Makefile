
CC=gcc

CFLAGS=-O3 -Wall -g -pg

all : uplink

uplink : LIBS+=-lfcgi

uplink : critbit.o uplink.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

%.pdf : %.tex
	@pdflatex $^
