# Compiling programming languages

This is schoolwork for Compiling Programming Languages.

In order to build:

```
$ mkdir build && cd build
$ cmake ..
$ make
```

In order to test:

```
$ make test
```

In order to use some of the sample programs:

```
$ echo "6 + 60 * 5 * cos(0)" | ./Tokenizer
Token(Number @ Span(0, 0), 6)
Token(Operator @ Span(0, 2), +)
Token(Number @ Span(0, 4), 60)
Token(Operator @ Span(0, 7), *)
Token(Number @ Span(0, 9), 5)
Token(Operator @ Span(0, 11), *)
Token(Identifier @ Span(0, 13), "cos")
Token(LeftParen @ Span(0, 16))
Token(Number @ Span(0, 17), 0)
Token(RightParen @ Span(0, 18))
Token(Eof @ Span(1, 0))<Paste>
```

```
$ echo "2 + 5 * 2 + cos (0)" | ./Evaluator
13
```
