CC := gcc

CFLAGS :=
LINKFLAGS := -lpthread -lrt

RW := reader_writer
PC := producer_consumer

PROGNAME := prog.c

EXEC_DIR := apps
EXEC_EXTENTION := out

.PHONY: all
all: producer_consumer reader_writer

.PHONY: producer_consumer
producer_consumer: $(PC)/$(PROGNAME)
	$(CC) $(CFLAGS) $< -o $@.$(EXEC_EXTENTION)

.PHONY: reader_writer
reader_writer: $(RW)/$(PROGNAME)
	$(CC) $(CFLAGS) $< -o $@.$(EXEC_EXTENTION)

.PHONY: clean
clean:
	rm -f *.$(EXEC_EXTENTION)
