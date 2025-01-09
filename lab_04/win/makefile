CC := gcc

CFLAGS :=
LINKFLAGS := -lpthread -lrt

PROGNAME := reader_writer.c

EXEC_EXTENTION := out

.PHONY: all
all: reader_writer

.PHONY: reader_writer
reader_writer: $(PROGNAME)
	$(CC) $(CFLAGS) $< -o $@.$(EXEC_EXTENTION)

.PHONY: clean
clean:
	rm -f *.$(EXEC_EXTENTION)
