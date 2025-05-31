CC = g++
CFLAGS = -Iinclude -std=c++17 -Wall -Wextra
SRC = src/main.cpp src/database/connection.cpp src/utils/config.cpp src/models/todo.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = cpp-sql-app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)