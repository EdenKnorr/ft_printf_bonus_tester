# ft_printf

Projeto da **École 42 Luanda** — reimplementação da função `printf()` da libc.

## 📋 Descrição

Este projeto consiste em recriar a função `printf()`, chamada `ft_printf()`,
sem usar a gestão de buffer da versão original. A biblioteca final é gerada
como `libftprintf.a`.

### Conversões suportadas (parte obrigatória)

| Conversão | Descrição                                      |
|-----------|-------------------------------------------------|
| `%c`      | Imprime um único caractere                       |
| `%s`      | Imprime uma string                               |
| `%p`      | Imprime um ponteiro `void *` em hexadecimal      |
| `%d`      | Imprime um número decimal (base 10)              |
| `%i`      | Imprime um inteiro em base 10                    |
| `%u`      | Imprime um número decimal sem sinal               |
| `%x`      | Imprime em hexadecimal (base 16), minúsculas     |
| `%X`      | Imprime em hexadecimal (base 16), maiúsculas     |
| `%%`      | Imprime o sinal de percentagem                   |

### Bónus implementados

- **Flags de largura e alinhamento**: `-` (alinha à esquerda) e `0`
  (preenche com zeros), além do campo de largura mínima (ex: `%10d`)
  e precisão (ex: `%.5d`), para todas as conversões.
- **Flags `#` e `+` e espaço (` `)**:
  - `+` força o sinal em números positivos
  - ` ` (espaço) imprime um espaço antes de números positivos
  - `#` antepõe `0x`/`0X` a valores hexadecimais não nulos

## ⚙️ Compilação

```bash
make          # compila a libftprintf.a
make clean    # remove os .o
make fclean   # remove .o e a lib
make re       # fclean + all
```

Ficheiros de entrega: `Makefile`, `*.h`, `*/*.h`, `*.c`, `*/*.c`
(regras obrigatórias do Makefile: `NAME`, `all`, `clean`, `fclean`, `re`).

O header `ft_printf.h` contém o protótipo:

```c
int ft_printf(const char *format, ...);
```

## 🧪 Testes (`main.c`)

O ficheiro `main.c` incluído neste repositório é um **testador automático**
que compara, teste a teste, o comportamento de `ft_printf` com o `printf`
original da libc — tanto o **texto produzido** como o **valor de retorno**.

Cobre a parte obrigatória e todas as flags do bónus, incluindo casos-limite
que um avaliador costuma explorar: `NULL` em `%s` e `%p`, `INT_MIN`,
precisão `0`, combinações de flags conflituantes, formato vazio, e uma
chamada com `format = NULL` só para garantir que não há segfault.

### Como correr

```bash
cc -Wall -Wextra -Werror main.c libftprintf.a -o test
./test
```

### Como interpretar o resultado

Cada linha mostra `[OK]` ou `[FAIL]`:

```
[OK]   d_int_min (ret=11) "-2147483648"
[FAIL] x_hash_flag
   ft_printf -> ret=4 | "0Xff"
   printf    -> ret=4 | "0xff"
```

No fim é impresso um resumo com o total de testes, quantos passaram e
quantos falharam. O objetivo é chegar a `0 FAIL` antes da avaliação.

## 👤 Autor

Projeto desenvolvido na École 42 Luanda.
