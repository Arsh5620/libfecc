# __MAKEFILE__

CC=gcc # C language compiler
# profile or debugging flags no longer required
# will be using vtune for profiling on Intel
# CFLAGS = -O0 -ggdb
CFLAGS = -O3 -flto

HEADERFILES=finite-fields.h rs.h polynomials.h
SOURCEFILES=rs.c finite-fields.c polynomials.c

EXEHEADERFILE=forwardecc.h 
EXEMAINFILE=forwardecc.c

LIBOBJFILES=${SOURCEFILES:c=o}
EXEOBJFILES=${EXEMAINFILE:c=o}
EXECUTABLE=main
LIBRARYNAME=libfecc.so

all: run-unit-tests build-library build-exe 

$(LIBOBJFILES): %.o: %.c $(HEADERFILES)
	$(CC) -c -fPIC $< -o $@  $(CFLAGS) -msse4

build-exe: $(LIBOBJFILES) $(EXEOBJFILES) build-library
	gcc $(CFLAGS) -o $(EXECUTABLE) $(EXEOBJFILES) -Wl,-R -Wl,./ $(LIBRARYNAME)
	
build-library: $(LIBOBJFILES)
	gcc $(CFLAGS) -shared -o $(LIBRARYNAME) $(LIBOBJFILES)

run-unit-tests:
	@echo "Starting unit tests before performing build..."
	cd ./test && make
	./test/main

clean:
	rm forwardecc.o finite-fields.o rs.o polynomials.o
