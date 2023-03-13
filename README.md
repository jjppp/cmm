# cmm
NJU Compiler Lab

cmm stands for C-- compiler.

# Project structure

```
Lab
├── Code
│   ├── Makefile
│   ├── lexical.l
│   ├── main.c
│   └── syntax.y
├── README.md
├── Test
│   ├── test1.cmm
│   └── test2.cmm
├── parser
└── report.pdf
```

## Code dir

1. Stores *.c *.l *.y and Makefile.
2. Complicated subdirectories are unnecessary.
3. Please do not modify the Makefile.
4. Please avoid unreasonable include dependencies.

## Test dir

1. Stores *.cmm test cases.
2. Uses ./parser to compile these tests.

## parser

Executable target file.

# TODO

## Analyses

- [X] Live Variables
- [X] Dominators
- [X] Reaching Definitions

## AST Opt

- [ ] Constant Folding

## Local Opt

- [X] Local Value Numbering

## Global Opt

- [X] Dead Code Elimination
- [X] Constant Propagation
- [X] Copy Propagation
- [X] Loop Invariant Code Motion

## Miscellaneous

- [X] Generic Set Container
- [ ] Better Comments
- [ ] Better Tests