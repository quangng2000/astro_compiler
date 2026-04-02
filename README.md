# Astro Compiler

A compiler for **Astro**, a statically-typed functional programming language with C/Rust-like syntax. The compiler features a full pipeline from source code to native machine code via **LLVM IR**.

## Language Features

- **Static type system** with explicit type annotations (`int`, `float`, `bool`, `string`, `void`)
- **Immutable bindings** by default (`let x = 5;`)
- **First-class functions** with type-annotated parameters and return types
- **Pattern matching** with `match` expressions and wildcard `_` support
- **If/else expressions** that return values
- **Operator precedence** via Pratt parsing (arithmetic, comparison, logical)
- **Line comments** with `//`
- **Built-in `print`** function for all types

## Example Program

```astro
fn factorial(n: int) -> int {
    match n {
        0 => 1,
        _ => n * factorial(n - 1),
    }
}

fn main() -> int {
    let result = factorial(5);
    print(result);
    return 0;
}
```

Output:

```
120
```

## Compiler Architecture

The compiler is organized into four phases, each as a separate module:

```
Source Code (.astro)
       |
       v
  +---------+
  |  Lexer  |  Tokenization (keywords, literals, operators)
  +---------+
       |
       v
  +---------+
  | Parser  |  Recursive descent + Pratt precedence -> AST
  +---------+
       |
       v
  +-------------+
  | Type Checker |  Static type analysis + TypeMap generation
  +-------------+
       |
       v
  +-----------+         +-----------+
  | LLVM      |   or    | C CodeGen |
  | CodeGen   |         |           |
  +-----------+         +-----------+
       |                      |
       v                      v
  LLVM IR (.ll)           C Source (.c)
       |                      |
       v                      v
  Object File (.o)        cc compiler
       |                      |
       v                      v
  Native Binary           Native Binary
```

### Phase Details

| Phase | Key Technique | Output |
|-------|---------------|--------|
| **Lexer** | Keyword table, multi-char operator lookahead | `std::vector<Token>` |
| **Parser** | Pratt binding power for expressions, recursive descent for declarations | Variant-based AST (`std::variant`) |
| **Type Checker** | Scoped symbol table, expression type propagation | `TypeMap` (expression -> type mapping) |
| **LLVM CodeGen** | IRBuilder, entry-block alloca pattern, PHI nodes for control flow | LLVM IR / object file |
| **C CodeGen** | String-based emission, ternary for expressions, GCC statement expressions | C source code |

## Building

### Prerequisites

- **CMake** 4.0+
- **C++26** compatible compiler (AppleClang 17+, GCC 14+, Clang 18+)
- **LLVM 22** (installed via Homebrew on macOS)

### Install LLVM (macOS)

```bash
brew install llvm
```

### Build

```bash
cmake -B build
cmake --build build
```

## Usage

```bash
# Default: compile to native binary via LLVM
./build/astro_compiler program.astro

# Emit LLVM IR for inspection
./build/astro_compiler program.astro --emit-llvm

# Use C codegen backend instead
./build/astro_compiler program.astro --emit-c
```

### Output Files

| Flag | Generated Files |
|------|----------------|
| *(default)* | `program.o` -> `program` (native binary) |
| `--emit-llvm` | `program.ll` (LLVM IR text) |
| `--emit-c` | `program.c` -> `program` (via system C compiler) |

## Language Reference

### Types

| Type | Description | Example |
|------|-------------|---------|
| `int` | 64-bit signed integer | `42`, `0`, `-1` |
| `float` | 64-bit double precision | `3.14`, `0.5` |
| `bool` | Boolean | `true`, `false` |
| `string` | String literal | `"hello"` |
| `void` | No return value | — |

### Variable Bindings

All bindings are immutable.

```astro
let x = 42;              // type inferred as int
let y: float = 3.14;     // explicit type annotation
let name = "astro";       // string
let flag: bool = true;    // boolean
```

### Functions

```astro
fn add(a: int, b: int) -> int {
    a + b
}

fn greet(name: string) -> void {
    print(name);
}
```

Functions can use an implicit return via a tail expression (last expression without semicolon) or an explicit `return` statement.

### If/Else Expressions

`if/else` blocks are expressions that return values:

```astro
fn abs(x: int) -> int {
    if x < 0 {
        0 - x
    } else {
        x
    }
}
```

### Pattern Matching

`match` expressions compare a value against patterns:

```astro
fn describe(n: int) -> int {
    match n {
        0 => 0,
        1 => 1,
        _ => 2,
    }
}
```

Supported patterns: integer literals, boolean literals, and wildcard `_`.

### Operators

Listed by precedence (highest to lowest):

| Precedence | Operators | Description |
|------------|-----------|-------------|
| 7 | `!`, `-` (unary) | Logical NOT, negation |
| 6 | `*`, `/`, `%` | Multiplication, division, modulo |
| 5 | `+`, `-` | Addition, subtraction |
| 4 | `<`, `>`, `<=`, `>=` | Comparison |
| 3 | `==`, `!=` | Equality |
| 2 | `&&` | Logical AND |
| 1 | `\|\|` | Logical OR |

### Built-in Functions

| Function | Description |
|----------|-------------|
| `print(value)` | Prints any value followed by a newline |

## Project Structure

```
astro-compiler/
├── CMakeLists.txt                    Build configuration
├── main.cpp                          CLI entry point and pipeline orchestration
├── src/
│   ├── common/
│   │   ├── source_location.h        Line/column tracking
│   │   ├── token.h                  TokenKind enum and Token struct
│   │   └── error.h                  CompilerError exception
│   ├── ast/
│   │   ├── ast_fwd.h                Forward declarations, Expr/Stmt variant aliases
│   │   ├── type_nodes.h             AstType enum and conversion helpers
│   │   ├── expr_nodes.h             Expression AST nodes (11 variants)
│   │   └── stmt_nodes.h             Statement nodes, FnDecl, Program
│   ├── lexer/
│   │   ├── lexer.h                  Lexer class declaration
│   │   ├── lexer.cpp                Tokenization (numbers, strings, identifiers)
│   │   └── lexer_helpers.cpp        Punctuation parsing, whitespace/comment skipping
│   ├── parser/
│   │   ├── parser.h                 Parser class declaration
│   │   ├── parser_expr.cpp          Pratt expression parsing
│   │   ├── parser_stmt.cpp          Statement and function declaration parsing
│   │   └── parser_helpers.cpp       Block parsing, precedence tables
│   ├── typechecker/
│   │   ├── type_env.h               TypeEnv (scoped symbol table) and FnSignature
│   │   ├── typechecker.h            TypeChecker class declaration
│   │   └── typechecker.cpp          Type checking and TypeMap generation
│   └── codegen/
│       ├── codegen.h                C CodeGen class declaration
│       ├── codegen_expr.cpp         C expression emission
│       ├── codegen_stmt.cpp         C function/statement emission
│       ├── llvm_codegen.h           LLVM CodeGen class declaration
│       ├── llvm_codegen.cpp         LLVM function/statement emission
│       ├── llvm_codegen_expr.cpp    LLVM expression emission (PHI nodes, SSA)
│       └── llvm_codegen_emit.cpp    LLVM IR output, object file, printf support
└── tests/
    ├── factorial.astro              Recursive factorial (pattern matching)
    └── features.astro               Arithmetic, if/else, match, booleans

27 source files | ~1,770 lines of C++ | every file under 100 lines
```

## Generated LLVM IR Example

For the factorial program above, the compiler generates:

```llvm
define i64 @factorial(i64 %n) {
entry:
  %n1 = alloca i64, align 8
  store i64 %n, ptr %n1, align 4
  %n2 = load i64, ptr %n1, align 4
  %0 = icmp eq i64 %n2, 0
  br i1 %0, label %match.arm, label %match.next

match.end:
  %1 = phi i64 [ 1, %match.arm ], [ %4, %match.next ]
  ret i64 %1

match.arm:
  br label %match.end

match.next:
  %n3 = load i64, ptr %n1, align 4
  %n4 = load i64, ptr %n1, align 4
  %2 = sub i64 %n4, 1
  %3 = call i64 @factorial(i64 %2)
  %4 = mul i64 %n3, %3
  br label %match.end
}
```

Key LLVM features used: SSA form, PHI nodes for pattern matching, entry-block allocas (mem2reg compatible), recursive function calls.

## Design Decisions

- **AST via `std::variant`** instead of inheritance — value-oriented, pattern-match with `std::visit`, no vtable overhead
- **Pratt parsing** for operator precedence — elegant, extensible, handles all binary/unary operators in ~60 lines
- **TypeMap bridge** (`std::unordered_map<const void*, AstType>`) — decouples type checker from code generator, allows multiple backends
- **Entry-block alloca pattern** — standard LLVM technique, works with mem2reg optimization pass to promote stack allocations to SSA registers
- **Dual backend** — LLVM for production-quality native code, C codegen retained as fallback and educational reference

## License

MIT
