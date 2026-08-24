/* ************************************************************************** */
/*                                                                            */
/*   main.c - Testador maluco para ft_printf (mandatory + bonus)             */
/*                                                                            */
/*   Compara o output e o valor de retorno do teu ft_printf com o printf     */
/*   original da libc, para cada teste.                                      */
/*                                                                            */
/*   Como compilar (ajusta o nome da tua lib/header se for diferente):       */
/*   cc -Wall -Wextra -Werror main.c libftprintf.a -o test                   */
/*   ./test                                                                  */
/*                                                                            */
/* ************************************************************************** */

/*
** Desativamos alguns avisos do GCC sobre o formato passado ao printf()
** original: o objetivo deste ficheiro E' testar combinacoes "proibidas"
** ou incomuns (NULL em %s, flags redundantes, etc.) contra o comportamento
** REAL da libc, entao o compilador nao deve bloquear a compilacao por isso.
*/
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
#pragma GCC diagnostic ignored "-Wformat-overflow"
#pragma GCC diagnostic ignored "-Wnonnull"

#include "ft_printf.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

static int g_total = 0;
static int g_fail = 0;

/*
** Compara o retorno e o output de ft_printf vs printf para os MESMOS
** argumentos. Como usamos macros, cada chamada de teste tem de passar
** os args duas vezes (uma para cada função) via TEST_INT/TEST_STR/etc.
** Para simplificar, comparamos manualmente escrevendo em buffers com
** snprintf-like approach: aqui comparamos diretamente na tela e o
** utilizador confere visualmente O RETURN e o output lado a lado.
**
** Para automação total, seria necessário redirecionar stdout para um
** buffer (dup2 + pipe), o que este arquivo também disponibiliza via
** a função capture_and_compare abaixo, usada nos testes críticos.
*/

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 65536

/*
** Executa duas funções (uma que chama ft_printf, outra que chama printf)
** capturando o stdout de cada uma, e compara byte a byte + valor de retorno.
*/
static void	check(const char *label, void (*call_ft)(int *ret, char *buf),
		void (*call_og)(int *ret, char *buf))
{
	char	buf_ft[BUF_SIZE];
	char	buf_og[BUF_SIZE];
	int		ret_ft;
	int		ret_og;

	g_total++;
	memset(buf_ft, 0, BUF_SIZE);
	memset(buf_og, 0, BUF_SIZE);
	call_ft(&ret_ft, buf_ft);
	call_og(&ret_og, buf_og);
	if (ret_ft != ret_og || strcmp(buf_ft, buf_og) != 0)
	{
		g_fail++;
		printf("\033[1;31m[FAIL]\033[0m %s\n", label);
		printf("   ft_printf -> ret=%d | \"%s\"\n", ret_ft, buf_ft);
		printf("   printf    -> ret=%d | \"%s\"\n", ret_og, buf_og);
	}
	else
	{
		printf("\033[1;32m[OK]\033[0m   %s (ret=%d) \"%s\"\n",
			label, ret_ft, buf_ft);
	}
}

/*
** Redireciona stdout para um pipe, chama a função alvo, lê o resultado
** para o buffer, e restaura stdout.
*/
static void	capture(int *ret, char *buf, int (*fn)(void *arg), void *arg)
{
	int		pipefd[2];
	int		saved_stdout;
	int		n;

	pipe(pipefd);
	saved_stdout = dup(STDOUT_FILENO);
	fflush(stdout);
	dup2(pipefd[1], STDOUT_FILENO);
	close(pipefd[1]);
	*ret = fn(arg);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);
	n = read(pipefd[0], buf, BUF_SIZE - 1);
	if (n < 0)
		n = 0;
	buf[n] = '\0';
	close(pipefd[0]);
}

/* ====================== WRAPPERS POR TESTE ====================== */
/* Cada teste define um par de funções (ft/og) que fazem a MESMA chamada */

#define DEFTEST(name, ft_call, og_call) \
	static int name##_ft(void *arg) { (void)arg; return (ft_call); } \
	static int name##_og(void *arg) { (void)arg; return (og_call); } \
	static void name##_wft(int *ret, char *buf) { capture(ret, buf, name##_ft, NULL); } \
	static void name##_wog(int *ret, char *buf) { capture(ret, buf, name##_og, NULL); }

/* ---------- %c ---------- */
DEFTEST(c_basic,        ft_printf("%c", 'A'),           printf("%c", 'A'))
DEFTEST(c_null_char,    ft_printf("[%c]", '\0'),        printf("[%c]", '\0'))
DEFTEST(c_width,        ft_printf("[%5c]", 'z'),        printf("[%5c]", 'z'))
DEFTEST(c_width_neg,    ft_printf("[%-5c]", 'z'),       printf("[%-5c]", 'z'))
DEFTEST(c_width_zero,   ft_printf("[%05c]", 'z'),       printf("[%05c]", 'z'))

/* ---------- %s ---------- */
DEFTEST(s_basic,        ft_printf("%s", "hello"),       printf("%s", "hello"))
DEFTEST(s_null,         ft_printf("[%s]", (char *)NULL), printf("[%s]", (char *)NULL))
DEFTEST(s_empty,        ft_printf("[%s]", ""),          printf("[%s]", ""))
DEFTEST(s_width,        ft_printf("[%10s]", "hi"),      printf("[%10s]", "hi"))
DEFTEST(s_width_neg,    ft_printf("[%-10s]", "hi"),     printf("[%-10s]", "hi"))
DEFTEST(s_precision,    ft_printf("[%.3s]", "hello"),   printf("[%.3s]", "hello"))
DEFTEST(s_prec_zero,    ft_printf("[%.0s]", "hello"),   printf("[%.0s]", "hello"))
DEFTEST(s_width_prec,   ft_printf("[%10.3s]", "hello"), printf("[%10.3s]", "hello"))
DEFTEST(s_width_prec_neg, ft_printf("[%-10.3s]", "hello"), printf("[%-10.3s]", "hello"))
DEFTEST(s_null_precision, ft_printf("[%.3s]", (char *)NULL), printf("[%.3s]", (char *)NULL))
DEFTEST(s_long,          ft_printf("%s", "this is a really long string to test buffer handling properly across boundaries maybe"), printf("%s", "this is a really long string to test buffer handling properly across boundaries maybe"))

/* ---------- %p ---------- */
DEFTEST(p_null,          ft_printf("%p", (void *)NULL), printf("%p", (void *)NULL))
DEFTEST(p_addr,           ft_printf("%p", (void *)0x1234abcd), printf("%p", (void *)0x1234abcd))
DEFTEST(p_stackaddr, ft_printf("%p", (void *)&g_total), printf("%p", (void *)&g_total))
DEFTEST(p_width,          ft_printf("[%15p]", (void *)0x42), printf("[%15p]", (void *)0x42))
DEFTEST(p_width_neg,      ft_printf("[%-15p]", (void *)0x42), printf("[%-15p]", (void *)0x42))

/* ---------- %d / %i ---------- */
DEFTEST(d_zero,          ft_printf("%d", 0),            printf("%d", 0))
DEFTEST(d_pos,            ft_printf("%d", 42),           printf("%d", 42))
DEFTEST(d_neg,            ft_printf("%d", -42),          printf("%d", -42))
DEFTEST(d_int_max,        ft_printf("%d", INT_MAX),      printf("%d", INT_MAX))
DEFTEST(d_int_min,        ft_printf("%d", INT_MIN),      printf("%d", INT_MIN))
DEFTEST(i_basic,          ft_printf("%i", -12345),       printf("%i", -12345))
DEFTEST(d_width,          ft_printf("[%8d]", 42),        printf("[%8d]", 42))
DEFTEST(d_width_neg_flag, ft_printf("[%-8d]", 42),       printf("[%-8d]", 42))
DEFTEST(d_zero_pad,       ft_printf("[%08d]", 42),       printf("[%08d]", 42))
DEFTEST(d_zero_pad_neg,   ft_printf("[%08d]", -42),      printf("[%08d]", -42))
DEFTEST(d_plus_flag,      ft_printf("[%+d]", 42),        printf("[%+d]", 42))
DEFTEST(d_plus_flag_neg,  ft_printf("[%+d]", -42),       printf("[%+d]", -42))
DEFTEST(d_space_flag,     ft_printf("[% d]", 42),        printf("[% d]", 42))
DEFTEST(d_space_flag_neg, ft_printf("[% d]", -42),       printf("[% d]", -42))
DEFTEST(d_precision,      ft_printf("[%.5d]", 42),       printf("[%.5d]", 42))
DEFTEST(d_precision_zero, ft_printf("[%.0d]", 0),        printf("[%.0d]", 0))
DEFTEST(d_prec_width,     ft_printf("[%8.5d]", 42),      printf("[%8.5d]", 42))
DEFTEST(d_prec_neg,       ft_printf("[%.5d]", -42),      printf("[%.5d]", -42))
DEFTEST(d_zero_prec_and_flag, ft_printf("[%08.5d]", 42), printf("[%08.5d]", 42))
DEFTEST(d_multi_flags,    ft_printf("[%+08d]", 42),      printf("[%+08d]", 42))
DEFTEST(d_huge_width,     ft_printf("[%20d]", -7),       printf("[%20d]", -7))

/* ---------- %u ---------- */
DEFTEST(u_zero,           ft_printf("%u", 0),            printf("%u", 0))
DEFTEST(u_basic,          ft_printf("%u", 42),           printf("%u", 42))
DEFTEST(u_neg_as_uint,    ft_printf("%u", -1),           printf("%u", -1))
DEFTEST(u_width,          ft_printf("[%10u]", 42),       printf("[%10u]", 42))
DEFTEST(u_zero_pad,       ft_printf("[%010u]", 42),      printf("[%010u]", 42))
DEFTEST(u_precision,      ft_printf("[%.6u]", 42),       printf("[%.6u]", 42))

/* ---------- %x / %X ---------- */
DEFTEST(x_zero,           ft_printf("%x", 0),            printf("%x", 0))
DEFTEST(x_basic,          ft_printf("%x", 255),          printf("%x", 255))
DEFTEST(X_basic,          ft_printf("%X", 255),          printf("%X", 255))
DEFTEST(x_neg,            ft_printf("%x", -1),           printf("%x", -1))
DEFTEST(x_width,          ft_printf("[%10x]", 255),      printf("[%10x]", 255))
DEFTEST(x_zero_pad,       ft_printf("[%010x]", 255),     printf("[%010x]", 255))
DEFTEST(x_precision,      ft_printf("[%.8x]", 255),      printf("[%.8x]", 255))
DEFTEST(x_hash_flag,      ft_printf("[%#x]", 255),       printf("[%#x]", 255))
DEFTEST(X_hash_flag,      ft_printf("[%#X]", 255),       printf("[%#X]", 255))
DEFTEST(x_hash_zero,      ft_printf("[%#x]", 0),         printf("[%#x]", 0))
DEFTEST(x_hash_width,     ft_printf("[%#10x]", 255),     printf("[%#10x]", 255))
DEFTEST(x_hash_zeropad,   ft_printf("[%#010x]", 255),    printf("[%#010x]", 255))
DEFTEST(x_prec_zero_val,  ft_printf("[%.0x]", 0),        printf("[%.0x]", 0))

/* ---------- %% ---------- */
DEFTEST(pct_basic,        ft_printf("%%"),               printf("%%"))
DEFTEST(pct_mixed,        ft_printf("100%% done"),       printf("100%% done"))

/* ---------- combinações malucas ---------- */
DEFTEST(mix_all,          ft_printf("%c-%s-%p-%d-%i-%u-%x-%X-%%", 'A', "str", (void *)0x10, -5, 5, 5, 255, 255),
                          printf("%c-%s-%p-%d-%i-%u-%x-%X-%%", 'A', "str", (void *)0x10, -5, 5, 5, 255, 255))
DEFTEST(mix_empty_fmt,    ft_printf(""),                 printf(""))
DEFTEST(mix_no_conv,      ft_printf("apenas texto sem conversao"), printf("apenas texto sem conversao"))
DEFTEST(mix_only_percent, ft_printf("%%%%%%"),           printf("%%%%%%"))
DEFTEST(mix_weird_order,  ft_printf("%-+08.5d", 42),     printf("%-+08.5d", 42))
DEFTEST(mix_all_flags_x,  ft_printf("[%-#10.6x]", 255),  printf("[%-#10.6x]", 255))
DEFTEST(mix_space_plus,   ft_printf("[% +d]", 5),        printf("[% +d]", 5))
DEFTEST(mix_plus_space,   ft_printf("[%+ d]", 5),        printf("[%+ d]", 5))
DEFTEST(mix_zero_dash,    ft_printf("[%0-8d]", 5),       printf("[%0-8d]", 5))
DEFTEST(mix_char_pct,     ft_printf("%c%%%c", 'x', 'y'), printf("%c%%%c", 'x', 'y'))
DEFTEST(mix_neg_width_prec_str, ft_printf("[%-15.4s]", "abcdefgh"), printf("[%-15.4s]", "abcdefgh"))
DEFTEST(mix_ptr_and_null_str, ft_printf("%p and %s", (void *)NULL, (char *)NULL),
                              printf("%p and %s", (void *)NULL, (char *)NULL))

/* ====================== MAIN ====================== */

int	main(void)
{
	printf("========== TESTES ft_printf vs printf ==========\n\n");

	printf("--- %%c ---\n");
	check("c_basic", c_basic_wft, c_basic_wog);
	check("c_null_char (deve so imprimir bytes nulos silenciosos)", c_null_char_wft, c_null_char_wog);
	check("c_width", c_width_wft, c_width_wog);
	check("c_width_neg", c_width_neg_wft, c_width_neg_wog);
	check("c_width_zero (zero-flag ignorada em %c no printf real)", c_width_zero_wft, c_width_zero_wog);

	printf("\n--- %%s ---\n");
	check("s_basic", s_basic_wft, s_basic_wog);
	check("s_null -> deve imprimir (null)", s_null_wft, s_null_wog);
	check("s_empty", s_empty_wft, s_empty_wog);
	check("s_width", s_width_wft, s_width_wog);
	check("s_width_neg", s_width_neg_wft, s_width_neg_wog);
	check("s_precision", s_precision_wft, s_precision_wog);
	check("s_prec_zero", s_prec_zero_wft, s_prec_zero_wog);
	check("s_width_prec", s_width_prec_wft, s_width_prec_wog);
	check("s_width_prec_neg", s_width_prec_neg_wft, s_width_prec_neg_wog);
	check("s_null_precision", s_null_precision_wft, s_null_precision_wog);
	check("s_long", s_long_wft, s_long_wog);

	printf("\n--- %%p ---\n");
	check("p_null", p_null_wft, p_null_wog);
	check("p_addr", p_addr_wft, p_addr_wog);
	check("p_stackaddr (enderecos vao diferir - confira so o formato/ret)", p_stackaddr_wft, p_stackaddr_wog);
	check("p_width", p_width_wft, p_width_wog);
	check("p_width_neg", p_width_neg_wft, p_width_neg_wog);

	printf("\n--- %%d / %%i ---\n");
	check("d_zero", d_zero_wft, d_zero_wog);
	check("d_pos", d_pos_wft, d_pos_wog);
	check("d_neg", d_neg_wft, d_neg_wog);
	check("d_int_max", d_int_max_wft, d_int_max_wog);
	check("d_int_min (cuidado com overflow de -INT_MIN!)", d_int_min_wft, d_int_min_wog);
	check("i_basic", i_basic_wft, i_basic_wog);
	check("d_width", d_width_wft, d_width_wog);
	check("d_width_neg_flag", d_width_neg_flag_wft, d_width_neg_flag_wog);
	check("d_zero_pad", d_zero_pad_wft, d_zero_pad_wog);
	check("d_zero_pad_neg", d_zero_pad_neg_wft, d_zero_pad_neg_wog);
	check("d_plus_flag", d_plus_flag_wft, d_plus_flag_wog);
	check("d_plus_flag_neg", d_plus_flag_neg_wft, d_plus_flag_neg_wog);
	check("d_space_flag", d_space_flag_wft, d_space_flag_wog);
	check("d_space_flag_neg", d_space_flag_neg_wft, d_space_flag_neg_wog);
	check("d_precision", d_precision_wft, d_precision_wog);
	check("d_precision_zero (0 com precisao 0 = string vazia!)", d_precision_zero_wft, d_precision_zero_wog);
	check("d_prec_width", d_prec_width_wft, d_prec_width_wog);
	check("d_prec_neg", d_prec_neg_wft, d_prec_neg_wog);
	check("d_zero_prec_and_flag (precisao anula o zero-flag)", d_zero_prec_and_flag_wft, d_zero_prec_and_flag_wog);
	check("d_multi_flags", d_multi_flags_wft, d_multi_flags_wog);
	check("d_huge_width", d_huge_width_wft, d_huge_width_wog);

	printf("\n--- %%u ---\n");
	check("u_zero", u_zero_wft, u_zero_wog);
	check("u_basic", u_basic_wft, u_basic_wog);
	check("u_neg_as_uint (-1 como unsigned)", u_neg_as_uint_wft, u_neg_as_uint_wog);
	check("u_width", u_width_wft, u_width_wog);
	check("u_zero_pad", u_zero_pad_wft, u_zero_pad_wog);
	check("u_precision", u_precision_wft, u_precision_wog);

	printf("\n--- %%x / %%X ---\n");
	check("x_zero", x_zero_wft, x_zero_wog);
	check("x_basic", x_basic_wft, x_basic_wog);
	check("X_basic", X_basic_wft, X_basic_wog);
	check("x_neg", x_neg_wft, x_neg_wog);
	check("x_width", x_width_wft, x_width_wog);
	check("x_zero_pad", x_zero_pad_wft, x_zero_pad_wog);
	check("x_precision", x_precision_wft, x_precision_wog);
	check("x_hash_flag (# antepoe 0x)", x_hash_flag_wft, x_hash_flag_wog);
	check("X_hash_flag (# antepoe 0X)", X_hash_flag_wft, X_hash_flag_wog);
	check("x_hash_zero (# NAO aparece se valor=0)", x_hash_zero_wft, x_hash_zero_wog);
	check("x_hash_width", x_hash_width_wft, x_hash_width_wog);
	check("x_hash_zeropad", x_hash_zeropad_wft, x_hash_zeropad_wog);
	check("x_prec_zero_val (0 com precisao 0 = vazio)", x_prec_zero_val_wft, x_prec_zero_val_wog);

	printf("\n--- %%%% ---\n");
	check("pct_basic", pct_basic_wft, pct_basic_wog);
	check("pct_mixed", pct_mixed_wft, pct_mixed_wog);

	printf("\n--- combinacoes malucas ---\n");
	check("mix_all", mix_all_wft, mix_all_wog);
	check("mix_empty_fmt", mix_empty_fmt_wft, mix_empty_fmt_wog);
	check("mix_no_conv", mix_no_conv_wft, mix_no_conv_wog);
	check("mix_only_percent", mix_only_percent_wft, mix_only_percent_wog);
	check("mix_weird_order", mix_weird_order_wft, mix_weird_order_wog);
	check("mix_all_flags_x", mix_all_flags_x_wft, mix_all_flags_x_wog);
	check("mix_space_plus (+ tem prioridade sobre espaco)", mix_space_plus_wft, mix_space_plus_wog);
	check("mix_plus_space", mix_plus_space_wft, mix_plus_space_wog);
	check("mix_zero_dash (dash tem prioridade sobre zero)", mix_zero_dash_wft, mix_zero_dash_wog);
	check("mix_char_pct", mix_char_pct_wft, mix_char_pct_wog);
	check("mix_neg_width_prec_str", mix_neg_width_prec_str_wft, mix_neg_width_prec_str_wog);
	check("mix_ptr_and_null_str", mix_ptr_and_null_str_wft, mix_ptr_and_null_str_wog);

	/* Teste manual de NULL como format (comportamento indefinido no printf real,
	   mas muitos avaliadores testam - o teu ft_printf NAO deve crashar) */
	printf("\n--- teste extra (NULL como format - so nao pode dar segfault) ---\n");
	printf("Chamando ft_printf(NULL) diretamente (sem comparar com printf, que teria UB):\n");
	{
		int ret = ft_printf(NULL);
		printf("ft_printf(NULL) retornou: %d (nao deve dar segfault)\n", ret);
	}

	printf("\n========== RESUMO ==========\n");
	printf("Total: %d | OK: %d | FAIL: %d\n", g_total, g_total - g_fail, g_fail);
	if (g_fail == 0)
		printf("\033[1;32mTODOS OS TESTES PASSARAM!\033[0m\n");
	else
		printf("\033[1;31m%d TESTE(S) FALHARAM.\033[0m\n", g_fail);
	return (0);
}
