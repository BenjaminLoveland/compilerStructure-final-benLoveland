CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

SRC = tokenizer.cpp parser.cpp main.cpp evaluator.cpp symbol_table.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = queryparser

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(EXEC)
	./$(EXEC)

clean:
	rm -f $(OBJ) $(EXEC)

rebuild: clean all

.PHONY: all run clean rebuild
