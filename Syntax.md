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

x = 5               \ Integer
y = 3.14            \ Float
name = "Dakota"    \ String
flag = true         \ Bool

## Literals

# Numbers
a = 10
b = 3.14

# Booleans
c = true
d = false

# Strings
msg = "Hello, World!"

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
