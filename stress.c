/* for wprintf() */
#define _ISOC99_SOURCE

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "aov-rx.h"

/* total number of tests and oks */
int tests = 0;
int oks = 0;

int verbose = 0;


/* failed tests messages */
char *failed_msgs[5000];
int i_failed_msgs = 0;

void _do_test(wchar_t *desc, wchar_t *rx, wchar_t *tx, wchar_t *exp_s, int src_line)
{
    wchar_t *stx, *stx2;
    int size;

    tests++;

    stx = aov_match(rx, tx, &size);
    stx2 = wcsdup(stx);
    stx2[size] = 0;

    if (wcscmp(stx2, exp_s) == 0) {
        /* match: test now the string */
        oks++;

        if (verbose)
            wprintf(L"stress.c:%d: OK (test %d, \"%ls\")\n", src_line, tests, desc);
    }
    else {
        wprintf(L"stress.c:%d: (test %d, \"%ls\") *** Failed ***", src_line, tests, desc);
        wprintf(L", size: %d, stx: \"%ls\", expected \"%ls\"\n", size, stx2, exp_s);
    }
}


int test_summary(void)
{
    wprintf(L"\n*** Total tests passed: %d/%d\n", oks, tests);

    if (oks == tests)
        wprintf(L"*** ALL TESTS PASSED\n");
    else {
//        int n;

        wprintf(L"*** %d %s\n", tests - oks, "TESTS ---FAILED---");

/*        printf("\nFailed tests:\n\n");
        for (n = 0; n < i_failed_msgs; n++)
            printf("%s", failed_msgs[n]);*/
    }

    return oks == tests ? 0 : 1;
}


#define do_test(d, r, t, s) _do_test(d, r, t, s, __LINE__)


wchar_t *match(wchar_t *rx, wchar_t *tx, int *size);


int main(int argc, char *argv[])
{
//    int r, o1, o2;

    if (argc > 1 && strcmp(argv[1], "-v") == 0)
        verbose = 1;

    do_test(L"basic 0", L".a", L"b", L"");
    do_test(L"basic 1", L".a$", L"b", L"");

    do_test(L"abcde", L"abcde", L"abc", L"abc");
    do_test(L"(abc)+ 1", L"(abc)+", L"abc", L"abc");
    do_test(L"(abc)+ 2", L"(abc)+", L"abcabc",L"abcabc");
    do_test(L"yes|no 1", L"yes|no", L"yes", L"yes");
    do_test(L"yes|no 2", L"yes|no", L"no", L"no");
    do_test(L"yes|you", L"yes|you", L"you", L"you");
    do_test(L"[ba0-9]+", L"[ba0-9]+", L"12a34c", L"12a34");
    do_test(L"[0-9]+", L"[0-9]+", L"12a34", L"12");
    do_test(L"test.*", L"test.*", L"test", L"test");
    do_test(L"test", L"test", L"test", L"test");
    do_test(L".*", L".*", L"", L"");

    /* ^ */
    do_test(L"Non-matching ^", L"^text", L"this string has text", L"");
    do_test(L"Matching ^", L"^this", L"this string has text", L"this");

    /* basic */
    do_test(L"Basic string", L"string", L"this string has text", L"string");
    do_test(L"Dots", L"h.la", L"hola", L"hola");

    /* * */
    do_test(L".* 1", L"g.*text", L"this string has text", L"g has text");
    do_test(L"i* 1", L"stri*ng", L"this string has text", L"string");
    do_test(L".* is non greedy", L"str.*ng", L"this string has string text", L"string");
    do_test(L"More than 1 .*", L"str.*ng.*x", L"this string has string text", L"string has string tex");
    do_test(L"* match to the end", L"one *world", L"one world", L"one world");
    do_test(L"More *", L"a*bc", L"aaabc", L"aaabc");

    /* ? */
    do_test(L"? 1", L"https?://", L"http://triptico.com", L"http://");
    do_test(L"? 2", L"https?://", L"https://triptico.com", L"https://");
    do_test(L"? 3", L"hos?la", L"hocla", L"");
    do_test(L"? 4", L"hos?la", L"hola", L"hola");
    do_test(L"? 5", L"hos?la", L"hosla", L"hosla");

    /* $ */
    do_test(L"Matching $ 1", L"text$", L"this string has text", L"text");
    do_test(L"Matching $ 2", L"text$", L"this string has text alone", L"");

    /* parens */
    do_test(L"Paren 1", L"(http|ftp)://",           L"http://triptico.com", L"http://");
    do_test(L"Paren 2", L"(http|ftp)://",           L"ftp://triptico.com", L"ftp://");
    do_test(L"Paren 3", L"(http|ftp)s?://",         L"http://triptico.com", L"http://");
    do_test(L"Paren 4", L"(http|ftp)s?://",         L"ftp://triptico.com", L"ftp://");
    do_test(L"Paren 5", L"(http|ftp)s?://",         L"ftps://triptico.com", L"ftps://");
    do_test(L"Paren 6", L"(gopher|http|ftp)s?://",  L"http://triptico.com", L"http://");
    do_test(L"Paren 7", L".(es|com)$",              L"http://triptico.com", L".com");
    do_test(L"Paren 8", L".(es|com)$",              L"http://triptico.es", L".es");
    do_test(L"Paren 9", L".(es|com)$",              L"http://triptico.org", L"");

    /* + */
    do_test(L"+ 0 (really *)", L"one *world",   L"oneworld is enough", L"oneworld");
    do_test(L"+ 1", L"one +world",              L"oneworld", L"");
    do_test(L"+ 2", L"one +world",              L"one world", L"one world");
    do_test(L"+ 3", L"one +world",              L"one    world", L"one    world");
    do_test(L"+ 4", L"one +world",              L"oneworld is enough", L"");
    do_test(L"+ 5", L"one +world",              L"one world is enough", L"one world");
    do_test(L"+ 6", L"one +world",              L"one    world is enough", L"one    world");
    do_test(L"+ 7", L"one +world",              L"I say oneworld is enough", L"");
    do_test(L"+ 8", L"one +world",              L"I say one world is enough", L"one world");
    do_test(L"+ 9", L"one +world",              L"I say one    world is enough", L"one    world");

    /* escaped chars */
    do_test(L"esc 0 (really ?)", L"ready?",         L"ready!", L"ready");
    do_test(L"esc 1",            L"ready\\?",       L"ready!", L"");
    do_test(L"esc 2",            L"ready\\?",       L"ready?", L"ready?");
    do_test(L"esc 3",            L"triptico.com",   L"tripticoxcom", L"tripticoxcom");
    do_test(L"esc 4",            L"triptico\\.com", L"tripticoxcom", L"");
    do_test(L"esc 5",            L"triptico\\.com", L"triptico.com", L"triptico.com");
    do_test(L"esc 6",            L"\n",             L"string without newlines", L"");
    do_test(L"esc 7",            L"\n",             L"I'm\nbroken", L"\n");

    /* square bracket sets */
    do_test(L"[] 0", L"[^a-c]",     L"z", L"z");
    do_test(L"[] 1", L"[^a-cdzx]",  L"z", L"");
    do_test(L"[] 2", L"[a-c]",      L"z", L"");
    do_test(L"[] 3", L"[a-cdzx]",   L"z", L"z");
    do_test(L"[] 4", L"[a-c]",      L"b", L"b");
    do_test(L"[] 5", L"[abc]",      L"b", L"b");
    do_test(L"[] 6", L"[abc]",      L"d", L"");

    do_test(L"[] and * 0", L"[a-z][a-z]*",  L"1234 string 456", L"string");
    do_test(L"[] and * 1", L"[a-z][a-z]*:", L"1234 string key: value 456", L"key:");
    do_test(L"[] and * 2", L"[a-c]*de",     L"abcde", L"abcde");

    do_test(L"[] and + 0", L"[a-z]+",       L"1234 string 456", L"string");
    do_test(L"[] and + 1", L"[a-z]+:",      L"1234 string key: value 456", L"key:");

    /* alternate strings */
    do_test(L"Alt strings 0", L"(abc|def)1", L"try abf1 now", L"");
    do_test(L"Alt strings 1", L"(abc|def)1", L"try def1 now", L"def1");
    do_test(L"Alt strings 2", L"(abc|def)1", L"try abc1 now", L"abc1");

    /* substrings */
#if 0
    do_test(L"Substrs and * 0", L"Rem(ark)* comment", L"Rem comment", L"Rem comment");
    do_test(L"Substrs and * 1", L"Rem(ark)* comment", L"Remarkark comment", L"Remarkark comment");
    do_test(L"Substrs and * 2", L"Rem(ark)* comment", L"Remark comment", L"Remark comment");
    do_test(L"Substrs and * 3", L"Rem(ark)* comment", L"<!-- Rem comment -->", L"Rem comment");
    do_test(L"Substrs and ? 0", L"Rem(ark)? comment", L"Rem comment", L"Rem comment");
    do_test(L"Substrs and ? 1", L"Rem(ark)? comment", L"Remark comment", L"Remark comment");
#endif

    do_test(L"More sets 0", L"'[^']*'", L"I have here a 'string' between quotes", L"'string'");
    do_test(L"More sets 1", L"'[^']*'", L"I have here a '' between quotes", L"''");
    do_test(L"More sets 2", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key:15 # comment", L"key:15");
    do_test(L"More sets 3", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key : 123456 # comment", L"key : 123456");
    do_test(L"More sets 4", L"[a-z][a-z]* *: *[1-9][0-9]*", L"k: 6000", L"k: 6000");
    do_test(L"More sets 5", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key: 1", L"key: 1");

    do_test(L"Brace matches 1 (like ?)", L"https{0,1}://",  L"http://triptico.com", L"http://");
    do_test(L"Brace matches 2 (like ?)", L"https{0,1}://",  L"https://triptico.com", L"https://");
    do_test(L"Brace matches 3 (like *)", L"a{0,}bc",        L"aaabc", L"aaabc");
    do_test(L"Brace matches 4 (like +)", L"one {1,}world",  L"oneworld", L"");
    do_test(L"Brace matches 5 (like +)", L"one {1,}world",  L"one world", L"one world");
    do_test(L"Brace matches 6 (like +)", L"one {1,}world",  L"one    world", L"one    world");

    do_test(L"Brace matches 11", L"a.{0,5}c",               L"abc", L"abc");
    do_test(L"Brace matches 12", L"a.{0,5}c",               L"abcdec", L"abc");
    do_test(L"Brace matches 13", L"a.{0,5}c",               L"abcdecfghic", L"abc");

    do_test(L"Brace matches 20", L".{5}",                   L"abcdefghijklmnopqrs", L"abcde");

    do_test(L"More * at the end", L"a*", L"", L"");

    return test_summary();
}
