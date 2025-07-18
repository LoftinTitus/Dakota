#!/usr/bin/env python3

types = [
    "INVALID",           # 0
    "INTEGER_LITERAL",   # 1
    "FLOAT_LITERAL",     # 2
    "STRING_LITERAL",    # 3
    "BOOLEAN_LITERAL",   # 4
    "IDENTIFIER",        # 5
    "BINARY_OP",         # 6
    "UNARY_OP",          # 7
    "ASSIGNMENT",        # 8
    "MATRIX_LITERAL",    # 9
    "MATRIX_MULTIPLY",   # 10
    "MATRIX_ACCESS",     # 11
    "ARRAY_ACCESS",      # 12
    "MEMBER_ACCESS",     # 13
    "IF_STATEMENT",      # 14
    "WHILE_STATEMENT",   # 15
    "FOR_STATEMENT",     # 16
    "FUNCTION_DEF",      # 17
    "FUNCTION_CALL",     # 18
    "RETURN_STATEMENT",  # 19
    "BLOCK",             # 20
    "EXPRESSION_STATEMENT", # 21
    "PROGRAM"            # 22
]

for i, t in enumerate(types):
    print(f"{i}: {t}")

print()
print(f"Type 8: {types[8]}")
print(f"Type 14: {types[14]}")
print(f"Type 21: {types[21]}")
