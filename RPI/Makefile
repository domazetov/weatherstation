IDIR = include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR = src

LIBS=-lm -lmosquitto

_DEPS = sensors.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = mqtt.o sensors.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mqtt: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f mqtt