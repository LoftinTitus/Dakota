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
INDENT_TEST_TARGET = $(BINDIR)/test_indentation
INTEGER_INDENT_TEST_TARGET = $(BINDIR)/test_integer_indent
BENCHMARK_TARGET = $(BINDIR)/benchmark_comments
OPTIMIZED_BENCHMARK_TARGET = $(BINDIR)/benchmark_optimized
PARSER_TEST_TARGET = $(BINDIR)/test_parser

.PHONY: all clean test test-indent test-integer-indent benchmark benchmark-optimized test-parser

all: $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

test-indent: $(INDENT_TEST_TARGET)
	./$(INDENT_TEST_TARGET)

test-integer-indent: $(INTEGER_INDENT_TEST_TARGET)
	./$(INTEGER_INDENT_TEST_TARGET)

benchmark: $(BENCHMARK_TARGET)
	./$(BENCHMARK_TARGET)

benchmark-optimized: $(OPTIMIZED_BENCHMARK_TARGET)
	./$(OPTIMIZED_BENCHMARK_TARGET)

test-parser: $(PARSER_TEST_TARGET)
	./$(PARSER_TEST_TARGET)

$(PARSER_TEST_TARGET): $(OBJDIR)/lexer.o $(OBJDIR)/parser.o $(OBJDIR)/test_parser.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OPTIMIZED_BENCHMARK_TARGET): $(OBJDIR)/lexer.o $(OBJDIR)/benchmark_optimized.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BENCHMARK_TARGET): $(OBJDIR)/lexer.o $(OBJDIR)/benchmark_comments.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(INTEGER_INDENT_TEST_TARGET): $(OBJDIR)/lexer.o $(OBJDIR)/test_integer_indent.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(INDENT_TEST_TARGET): $(OBJDIR)/lexer.o $(OBJDIR)/test_indentation.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

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
$(OBJDIR)/parser.o: $(SRCDIR)/parser.cpp $(SRCDIR)/parser.h $(SRCDIR)/lexer.h
$(OBJDIR)/test_lexer.o: $(SRCDIR)/test_lexer.cpp $(SRCDIR)/lexer.h
$(OBJDIR)/test_indentation.o: $(SRCDIR)/test_indentation.cpp $(SRCDIR)/lexer.h
$(OBJDIR)/test_integer_indent.o: $(SRCDIR)/test_integer_indent.cpp $(SRCDIR)/lexer.h
$(OBJDIR)/benchmark_comments.o: $(SRCDIR)/benchmark_comments.cpp $(SRCDIR)/lexer.h
$(OBJDIR)/benchmark_optimized.o: $(SRCDIR)/benchmark_optimized.cpp $(SRCDIR)/lexer.h
$(OBJDIR)/test_parser.o: $(SRCDIR)/test_parser.cpp $(SRCDIR)/parser.h $(SRCDIR)/lexer.h
