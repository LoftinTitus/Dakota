CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Targets
TARGET = $(BINDIR)/dakota
TEST_TARGET = $(BINDIR)/test_lexer

.PHONY: all clean test

all: $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(OBJDIR)/lexer.o $(OBJDIR)/test_lexer.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TARGET): $(filter-out $(OBJDIR)/test_lexer.o, $(OBJECTS)) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Dependencies
$(OBJDIR)/lexer.o: $(SRCDIR)/lexer.cpp $(SRCDIR)/lexer.h
$(OBJDIR)/test_lexer.o: $(SRCDIR)/test_lexer.cpp $(SRCDIR)/lexer.h
