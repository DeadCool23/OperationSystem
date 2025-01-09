CC := gcc

CFLAGS :=
LINKFLAGS :=

PROGNAME := reader_writer.c

EXEC_EXTENTION := exe

.PHONY: all
all: reader_writer

.PHONY: reader_writer
reader_writer: $(PROGNAME)
	$(CC) $(CFLAGS) $< -o $@.$(EXEC_EXTENTION)

.PHONY: clean
clean:
	rm -f *.$(EXEC_EXTENTION)
