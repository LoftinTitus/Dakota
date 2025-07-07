# Dakota

**Dakota** is a compiled, high-performance programming language designed for engineers, scientists, and technical computing. It combines Python-like readability with the raw speed of compiled languages, providing first-class support for linear algebra and numerical operations.

Inspired by the simplicity of Python and the numerical power of MATLAB, Dakota aims to be the ideal tool for building simulations, solving engineering problems, and performing scientific computations with clean, intuitive syntax.

---

## Project Goals

- Readable, Python-like syntax
- Native, efficient support for:
  - Vectors, matrices, tensors
  - Matrix multiplication, transposition, inversion
  - Linear algebra operations at the language level
- Compiled for high performance (via LLVM)
- Ideal for engineering, math, and scientific computing
- Minimal learning curve for users familiar with Python or MATLAB

---

## Example (Planned Syntax)

```
# Vector and matrix declarations
x = [1;2;3]
A = [1,0,0;0,1,0;0,0,1]

# Matrix-vector multiplication
y = A @ x

# Transpose and addition
B = A.T + A

# Print result
print(y)
```

# TODO List for EngLang

## Language Design
- [X] Finalize core syntax (variables, math, matrix operations)
- [X] Design type system (scalar, vector, matrix, etc.)
- [X] Define standard library scope (math, linear algebra)

##  Compiler Development
- [ ] Set up C++ project with CMake
- [X] Implement lexer (tokenizer)
- [ ] Build parser and AST structures
- [ ] Integrate LLVM for code generation
- [ ] Wrap Eigen or BLAS/LAPACK for matrix operations

## Testing & Examples
- [ ] Write basic example programs in `examples/`
- [ ] Create unit tests for compiler components
- [ ] Validate matrix operations and math functionality

## Documentation
- [ ] Write `docs/syntax.md` with language grammar and examples
- [ ] Document build instructions
- [ ] Draft language philosophy and vision

## Nice to Have
- [ ] Units of measurement system (e.g., `5 * m`, unit-safe math)
- [ ] Complex numbers and advanced math support
- [ ] Optional JIT compilation mode for fast prototyping
- [ ] Hardware acceleration for matrix operations (SIMD/GPU)
