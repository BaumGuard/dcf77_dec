CC = gcc
CFLAGS = -c `pkg-config --cflags gtk+-3.0`
LDFLAGS = -lm -lasound `pkg-config --libs gtk+-3.0`

TARGET = dcf77_dec

SRCDIR = src
BUILDDIR = build

all: $(TARGET)

$(TARGET): $(BUILDDIR)/main.o $(BUILDDIR)/filter.o $(BUILDDIR)/input.o  $(BUILDDIR)/decoder.o $(BUILDDIR)/buffer.o $(BUILDDIR)/demod.o $(BUILDDIR)/common.o $(BUILDDIR)/gui.o $(BUILDDIR)/event.o $(BUILDDIR)/utils.o
	$(CC) $(LDFLAGS) $(BUILDDIR)/*.o -o $(TARGET)

$(BUILDDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c -o $(BUILDDIR)/main.o

$(BUILDDIR)/filter.o: $(SRCDIR)/filter.c $(SRCDIR)/filter.h
	$(CC) $(CFLAGS) $(SRCDIR)/filter.c -o $(BUILDDIR)/filter.o

$(BUILDDIR)/common.o: $(SRCDIR)/common.c $(SRCDIR)/common.h
	$(CC) $(CFLAGS) $(SRCDIR)/common.c -o $(BUILDDIR)/common.o

$(BUILDDIR)/input.o: $(SRCDIR)/input.c $(SRCDIR)/input.h
	$(CC) $(CFLAGS) $(SRCDIR)/input.c -o $(BUILDDIR)/input.o

$(BUILDDIR)/decoder.o: $(SRCDIR)/decoder.c $(SRCDIR)/decoder.h
	$(CC) $(CFLAGS) $(SRCDIR)/decoder.c -o $(BUILDDIR)/decoder.o

$(BUILDDIR)/buffer.o: $(SRCDIR)/buffer.c $(SRCDIR)/buffer.h
	$(CC) $(CFLAGS) $(SRCDIR)/buffer.c -o $(BUILDDIR)/buffer.o

$(BUILDDIR)/demod.o: $(SRCDIR)/demod.c $(SRCDIR)/demod.h
	$(CC) $(CFLAGS) $(SRCDIR)/demod.c -o $(BUILDDIR)/demod.o

$(BUILDDIR)/gui.o: $(SRCDIR)/gui.c $(SRCDIR)/gui.h
	$(CC) $(CFLAGS) $(SRCDIR)/gui.c -o $(BUILDDIR)/gui.o

$(BUILDDIR)/event.o: $(SRCDIR)/event.c $(SRCDIR)/event.h
	$(CC) $(CFLAGS) $(SRCDIR)/event.c -o $(BUILDDIR)/event.o

$(BUILDDIR)/utils.o: $(SRCDIR)/utils.c $(SRCDIR)/utils.h
	$(CC) $(CFLAGS) $(SRCDIR)/utils.c -o $(BUILDDIR)/utils.o

clean:
	rm -rf dcf77_dec $(BUILDDIR)/*.o

install:
	mkdir /usr/share/dcf77_dec
	scp $(TARGET) /usr/bin
	scp dcf77_dec.glade /usr/share/dcf77_dec

uninstall:
	rm /usr/bin/$(TARGET)
	rm -r /usr/share/dcf77_dec
