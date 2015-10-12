CC = gcc
CFLAGS = -std=c99 -O2 -Wall -finline-functions -Iinclude
AR = ar
AR_FLAGS = -cq
LIB_FILE = libsat.a
LIB_DES1 = ../sat_solver/lib/libsat.a
LIB_DES2= ../c2D_code/lib/darwin/libsat.a

SRC = src/sat_api.c src/tools.c

OBJS=$(SRC:.c=.o)

HEADERS = include/sat_api.h
HEADERS_DES = ../sat_solver/include/satapi.h

sat: $(OBJS)
	$(AR) $(AR_FLAGS) $(LIB_FILE) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB_FILE)

copy: sat
	cp $(LIB_FILE) $(LIB_DES1)
	cp $(LIB_FILE) $(LIB_DES2)
	cp ./$(HEADERS) ../sat_solver/$(HEADERS)
