CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lpthread -lrt

TARGETS = logger writer reader
SOURCES = logger.c writer.c reader.c
OBJECTS = logger.o writer.o reader.o

# Phony targets
.PHONY: all clean run test help

# Default target
all: $(TARGETS)

# Compile logger
logger: logger.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "✅ Compiled: $@"

# Compile writer
writer: writer.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "✅ Compiled: $@"

# Compile reader
reader: reader.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "✅ Compiled: $@"

# Object files
logger.o: logger.c
	$(CC) $(CFLAGS) -c $<

writer.o: writer.c
	$(CC) $(CFLAGS) -c $<

reader.o: reader.c
	$(CC) $(CFLAGS) -c $<

# Clean
clean:
	@rm -f $(OBJECTS) $(TARGETS)
	@echo "🗑️  Cleaned: $(OBJECTS) $(TARGETS)"

# Help
help:
	@echo "========================================"
	@echo "Shared Memory & Semaphore Makefile"
	@echo "========================================"
	@echo "make          - Compile all programs"
	@echo "make clean    - Remove compiled files"
	@echo "make help     - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  Terminal 1: ./logger"
	@echo "  Terminal 2: ./reader"
	@echo "  Terminal 3: ./writer"
	@echo "========================================"

# Test rule
test: all
	@echo ""
	@echo "Testing system..."
	@./logger &
	@LOGGER_PID=$$!; \
	sleep 1; \
	./reader &  \
	READER_PID=$$!; \
	sleep 1; \
	(echo "Test message 1"; sleep 1; echo "Test message 2"; sleep 1) | ./writer; \
	sleep 2; \
	kill $$LOGGER_PID $$READER_PID 2>/dev/null || true; \
	echo ""; \
	echo "📄 Log file contents:"; \
	cat /tmp/example.log; \
	echo ""
