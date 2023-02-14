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
