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

## Lab2

- [ ] 变量在使用时未经定义。
- [ ] 函数在调用时未经定义。
- [ ] 变量出现重复定义，或变量与前面定义过的结构体名字重复。
- [ ] 函数出现重复定义（即同样的函数名出现了不止一次定义）。
- [ ] 赋值号两边的表达式类型不匹配。
- [ ] 赋值号左边出现一个只有右值的表达式。
- [ ] 操作数类型不匹配或操作数类型与操作符不匹配（例如整型变量与数组变量相加减，或数组（或结构体）变量与数组（或结构体）变量相加减）。
- [ ] return语句的返回类型与函数定义的返回类型不匹配。
- [ ] 函数调用时实参与形参的数目或类型不匹配。
- [ ] 对非数组型变量使用“[...]”（数组访问）操作符。
- [ ] 对普通变量使用“(...)”或“()”（函数调用）操作符。
- [ ] 数组访问操作符“[...]”中出现非整数（例如a[1.5]）。
- [ ] 对非结构体型变量使用“.”操作符。
- [ ] 访问结构体中未定义过的域。
- [ ] 结构体中域名重复定义（指同一结构体中），或在定义时对域进行初始化（例如struct A { int a = 0; }）。
- [ ] 结构体的名字与前面定义过的结构体或变量的名字重复。
- [ ] 直接使用未定义过的结构体来定义变量。