#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options] <source_file>\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -i, --interactive  Start interactive mode (REPL)\n";
    std::cout << "  -c <code>      Execute code string directly\n";
    std::cout << "  -p, --parse-only   Parse only, don't execute\n";
    std::cout << "  -v, --verbose      Verbose output\n";
}

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

void run_code(const std::string& code, bool parse_only = false, bool verbose = false) {
    try {
        if (verbose) {
            std::cout << "=== Lexing ===\n";
        }
        
        // Tokenize
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        if (verbose) {
            std::cout << "Generated " << tokens.size() << " tokens\n";
        }
        
        if (verbose) {
            std::cout << "=== Parsing ===\n";
        }
        
        // Parse
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cerr << "Parse error: " << parser.get_error() << std::endl;
            return;
        }
        
        if (verbose) {
            std::cout << "Generated " << parser.get_nodes().size() << " AST nodes\n";
            std::cout << "Memory usage: " << parser.get_memory_usage() << " bytes\n";
        }
        
        if (parse_only) {
            std::cout << "Parsing completed successfully.\n";
            if (verbose) {
                std::cout << "\nAST Structure:\n";
                parser.print_ast(0);
            }
            return;
        }
        
        if (verbose) {
            std::cout << "=== Interpreting ===\n";
        }
        
        // Interpret
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
    } catch (const Dakota::RuntimeError& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void interactive_mode() {
    std::cout << "Dakota Interactive Mode\n";
    std::cout << "Type 'exit' or 'quit' to exit, 'help' for help\n\n";
    
    std::string line;
    int line_number = 1;
    
    while (true) {
        std::cout << "dakota:" << line_number << "> ";
        std::getline(std::cin, line);
        
        if (line == "exit" || line == "quit") {
            std::cout << "Goodbye!\n";
            break;
        }
        
        if (line == "help") {
            std::cout << "Available commands:\n";
            std::cout << "  exit, quit  - Exit the interpreter\n";
            std::cout << "  help        - Show this help\n";
            std::cout << "  Any Dakota code to execute\n\n";
            std::cout << "Example Dakota code:\n";
            std::cout << "  x = 42\n";
            std::cout << "  y = 3.14\n";
            std::cout << "  matrix = [1, 2; 3, 4]\n";
            std::cout << "  print(x + y)\n";
            std::cout << "  print(matrix)\n\n";
            continue;
        }
        
        if (line.empty()) {
            continue;
        }
        
        run_code(line);
        line_number++;
    }
}

int main(int argc, char* argv[]) {
    bool interactive = false;
    bool parse_only = false;
    bool verbose = false;
    std::string code_string;
    std::string filename;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-i" || arg == "--interactive") {
            interactive = true;
        } else if (arg == "-p" || arg == "--parse-only") {
            parse_only = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-c") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -c option requires a code string\n";
                return 1;
            }
            code_string = argv[++i];
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        } else {
            filename = arg;
        }
    }
    
    try {
        if (interactive) {
            interactive_mode();
        } else if (!code_string.empty()) {
            run_code(code_string, parse_only, verbose);
        } else if (!filename.empty()) {
            std::string code = read_file(filename);
            run_code(code, parse_only, verbose);
        } else {
            std::cerr << "Error: No input provided\n";
            print_usage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
