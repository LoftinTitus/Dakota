# Dakota Syntax Specification

This document defines the proposed syntax, structure, and core features of **Dakota**, a compiled, high-performance programming language for engineers and scientists. The syntax is designed to be highly readable, familiar to Python users, and optimized for mathematical and linear algebra operations.

---

## Table of Contents

- [General Language Structure](#general-language-structure)
- [Comments](#comments)
- [Variables and Types](#variables-and-types)
- [Literals](#literals)
- [Expressions and Operators](#expressions-and-operators)
- [Vectors and Matrices](#vectors-and-matrices)
- [Matrix Operations](#matrix-operations)
- [Functions](#functions)
- [Control Flow](#control-flow)
- [Built-in Functions](#built-in-functions)
- [Error Handling (Future)](#error-handling-future)
- [Notes on Static Typing](#notes-on-static-typing)
- [Future Considerations](#future-considerations)

---

## General Language Structure

- **Whitespace and Indentation** define code blocks
- The language is **case-sensitive**
- All code is executed top-down unless inside functions or control structures

---

## Comments

Single-line comments use `\`

```
\ This is a comment
x = 5  \ Inline comment
```

## Variables
Variables are declared implicitly by assignment. The compiler infers types where possible, but types are strictly enforced for performance.
```
x = 5               \ Integer
y = 3.14            \ Float
name = "Dakota"    \ String
flag = true         \ Bool
```
## Literals

# Numbers
```
a = 10
b = 3.14
```

# Booleans
```
c = true
d = false
```
# Strings
```
msg = "Hello, World!"
```
## Expressions and operators

# Arithmetic
```
+   \ Additon
-   \ Substraction
*   \ Multiplication
/   \ Division
%   \ Modulo
**   \ Exponent
```

# Comparison
```
==   \ Equal to
!=   \ Not equal to
<   \ Greater/Less than
>   \ Less/Greater than
<=  \ Greater/Less than or equal to
and    \ and
or   \ or
not   \ not
```

# Linear algebra
Semi-colons skip to the next row, while commas skip to the next column
```
Column vector: [1;1;1] 
matrix: [1,0,0;0,1,0;0,0,1]

A mult x   \ A multiplied by x
A.I   \ Inverse of A
A.T   \ Transpose of A
A.d   \ Determinant of A
A.tr    \ Trace of A
```

# Function declarations
Functions are defined using "func"
func add(a, b):
    return a + b

# Control flow
If/else statements:
```
x = 5

if x > 3:
    print("Greater")
else:
    print("Smaller or equal")
```

While loops:
```
i = 0

while i < 5:
    print(i)
    i = i + 1
```

For loops:
```
for i in 0..5:
    print(i)
```

# Built in functions
```
see()   \ displays to terminal/console/etc.
```

# Error handling
If an unrecoverable error occurs, the program terminates with a descriptive message:
```
A = [[1, 2], [3, 4]]
B = [1, 2, 3]

C = A @ B   # Runtime error: Incompatible dimensions
```
Console output:

Error: Matrix dimensions do not align for multiplication (2x2 @ 3x1)

You can attempt risky operations with try:
```
try:
    AI = inv(A)   # Try to invert matrix
    print(AI)
except:
    print("Matrix is singular, cannot invert")

result = try inv(A)

if result.is_error:
    print("Failed to invert matrix")
else:
    print(result.value)
```

The complier should catch:
```
x = 5
y = "hello"

z = x + y   # Compile-time error: Cannot add int and string
```


# Static typing
All variables will have a fixed type at compile. 
Type inference reduces need for explicit annotations.
Compiler enforces type safety for performance and correctness

# Future considerations
A unit declaration system to simplify calculations and prevent errors:
```
length = 5 [m]
time = 2 [s]
velocity = length / time  # Units tracked automatically

force = mass * acceleration
```

Complex number support:
```
z = 3 + 4i
magnitude = abs(z)
conjugate = z.conj()
```

Native plotting to make it easier to visulize data:

x = linspace(0, 10, 100)
y = sin(x)

plt(x, y)
