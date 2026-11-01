# ==============================================================================
# LifeLink - Blood Donor Finder Makefile
# C++17, Crow Web Framework, SQLite3 (Cross-platform: Linux & Windows)
# ==============================================================================

CXX ?= g++
CC ?= gcc
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude
CFLAGS = -O2 -Iinclude

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32 -lmswsock
    TARGET = lifelink_server.exe
    RM = del /f /q
else
    LDFLAGS = -lpthread -ldl
    TARGET = lifelink_server
    RM = rm -f
endif

OBJS = db_manager.o server.o sqlite3.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "Build successful: $(TARGET)"

server.o: server.cpp db_manager.hpp
	$(CXX) $(CXXFLAGS) -c server.cpp -o server.o

db_manager.o: db_manager.cpp db_manager.hpp
	$(CXX) $(CXXFLAGS) -c db_manager.cpp -o db_manager.o

sqlite3.o: sqlite3.c
	$(CC) $(CFLAGS) -c sqlite3.c -o sqlite3.o

clean:
	$(RM) server.o db_manager.o $(TARGET) 2>/dev/null || $(RM) server.o db_manager.o $(TARGET) 2>nul || true

run: all
	./$(TARGET)

.PHONY: all clean run
