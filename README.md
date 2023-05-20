# compiler2023spring

Course project of Compiler Principles (spring 2023).

Heartfelt appreciation for the effort of the kind TA MaxXing and Prof. Liu,
put in designing and organizing this course project, the Koopa IR and library,
and the well-written documentation.

## Versions

- 2023.05.20: A roughly working version. All local tests passed.

## Code organization

### Frontend

The first half of `main.cpp`.
- `ast.h` and `sysy.l/y` work together, with `type.h` and `symbol.h`.
    - `sysy.l/y`: SysY to AST.
    - `ast.h`: AST to text-form Koopa IR.

### Backend

The second half of `main.cpp`.
- Use libkoopa (`koopa.h`) to convert text-form Koopa IR into memory-form.
- In `koopair.cpp/h`, convert Koopa IR to RISCV, with `frame.h`, `riscv.h`
and `array.h`.
