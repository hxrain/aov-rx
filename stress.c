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
    size_t size;

    tests++;

    stx = aov_match(rx, tx, &size);

    if (stx && *stx) {
        stx2 = wcsdup(stx);

        if (wcslen(stx2) < size)
            wprintf(L"stress.c:%d: warning: wcslen(stx2) %d < size %d (stx2: %ls)\n",
                src_line, wcslen(stx2), size, stx2);
        else
            stx2[size] = 0;
    }
    else
        stx2 = L"";

    if (wcscmp(stx2, exp_s) == 0) {
        /* match: test now the string */
        oks++;

        if (verbose)
            wprintf(L"stress.c:%d: OK (test %d, \"%ls\")\n", src_line, tests, desc);
    }
    else {
        wprintf(L"stress.c:%d: error: (test %d, \"%ls\") ", src_line, tests, desc);
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


struct rx_test {
    wchar_t *rx;
    wchar_t *tx;
    wchar_t *r;
    int src_line;
} rx_tests[] = {
    { L".*a",       L"a",                                 L"a", __LINE__ },
    { L"a",         L"a",                                 L"a", __LINE__ },
    { L"a*",        L"a",                                 L"a", __LINE__ },
    { L"a+",        L"a",                                 L"a", __LINE__ },
    { L"a.*",       L"a",                                 L"a", __LINE__ },
    { L".a",        L"b",                                 L"", __LINE__ },
    { L".a$",       L"b",                                 L"", __LINE__ },

    { L"abc",       L"abcde",                             L"abc", __LINE__ },
    { L"(abc)+",    L"abc",                               L"abc", __LINE__ },
    { L"(abc)+",    L"abcabc",                            L"abcabc", __LINE__ },
    { L"yes|no",    L"yes",                               L"yes", __LINE__ },
    { L"yes|no",    L"no",                                L"no", __LINE__ },
    { L"yes|you",   L"you",                               L"you", __LINE__ },
    { L"(yes|no)",  L"yes",                               L"yes", __LINE__ },
    { L"(yes|no)",  L"no",                                L"no", __LINE__ },
    { L"(yes|you)", L"you",                               L"you", __LINE__ },
    { L"[ba0-9]+",  L"12a34c",                            L"12a34", __LINE__ },
    { L"[0-9]+",    L"12a34",                             L"12", __LINE__ },
    { L"test.*",    L"test",                              L"test", __LINE__ },
    { L"test",      L"test",                              L"test", __LINE__ },
    { L".*",        L"",                                  L"", __LINE__ },

    /* ^ */
    { L"^text",     L"this string has text",              L"", __LINE__ },
    { L"^this",     L"this string has text",              L"this", __LINE__ },

    /* basic */
    { L"string",    L"this string has text",              L"string", __LINE__ },
    { L"h.la",      L"hola",                              L"hola", __LINE__ },

    /* * */
    { L"g.*text",   L"this string has text",              L"g has text", __LINE__ },
    { L"stri*ng",   L"this string has text",              L"string", __LINE__ },

    /* .* non-greedy */
    { L"str.*ng",    L"this string has string text",      L"string", __LINE__ },
    { L"str.*ng.*x", L"this string has string text",      L"string has string tex", __LINE__ },
    { L"one *world", L"one world",                        L"one world", __LINE__ },
    { L"a*bc",       L"aaabc",                            L"aaabc", __LINE__ },

    /* ? */
    { L"https?://",  L"http://triptico.com",              L"http://", __LINE__ },
    { L"https?://",  L"https://triptico.com",             L"https://", __LINE__ },
    { L"hos?la",     L"hocla",                            L"", __LINE__ },
    { L"hos?la",     L"hola",                             L"hola", __LINE__ },
    { L"hos?la",     L"hosla",                            L"hosla", __LINE__ },

    /* $ */
    { L"text$",      L"this string has text",             L"text", __LINE__ },
    { L"text$",      L"this string has text alone",       L"", __LINE__ },

    /* parens */
    { L"(http|ftp)://",     L"http://triptico.com",       L"http://", __LINE__ },
    { L"(http|ftp)://",     L"ftp://triptico.com",        L"ftp://", __LINE__ },
    { L"(http|ftp)s?://",   L"http://triptico.com",       L"http://", __LINE__ },
    { L"(http|ftp)s?://",   L"ftp://triptico.com",        L"ftp://", __LINE__ },
    { L"(http|ftp)s?://",   L"ftps://triptico.com",       L"ftps://", __LINE__ },
    { L"(gopher|http|ftp)s?://",  L"http://triptico.com", L"http://", __LINE__ },
    { L".(es|com)$",        L"http://triptico.com",       L".com", __LINE__ },
    { L".(es|com)$",        L"http://triptico.es",        L".es", __LINE__ },
    { L".(es|com)$",        L"http://triptico.org",       L"", __LINE__ },

    { NULL,         NULL,                                 NULL, 0 }
};


int main(int argc, char *argv[])
{
    int n;

    if (argc > 1 && strcmp(argv[1], "-v") == 0)
        verbose = 1;

    /* loop all tests */
    for (n = 0; rx_tests[n].rx != NULL; n++) {
        struct rx_test *r = &rx_tests[n];

        _do_test(r->rx, r->rx, r->tx, r->r, r->src_line);
    }

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
    do_test(L"Substrs and * 0", L"Rem(ark)* comment", L"Rem comment", L"Rem comment");
    do_test(L"Substrs and * 1", L"Rem(ark)* comment", L"Remarkark comment", L"Remarkark comment");
    do_test(L"Substrs and * 2", L"Rem(ark)* comment", L"Remark comment", L"Remark comment");
    do_test(L"Substrs and * 3", L"Rem(ark)* comment", L"<!-- Rem comment -->", L"Rem comment");
    do_test(L"Substrs and ? 0", L"Rem(ark)? comment", L"Rem comment", L"Rem comment");
    do_test(L"Substrs and ? 1", L"Rem(ark)? comment", L"Remark comment", L"Remark comment");

    do_test(L"More sets 0", L"'[^']*'", L"I have here a 'string' between quotes", L"'string'");
    do_test(L"More sets 1", L"'[^']*'", L"I have here a '' between quotes", L"''");
    do_test(L"More sets 2", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key:15 # comment", L"key:15");
    do_test(L"More sets 3", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key : 123456 # comment", L"key : 123456");
    do_test(L"More sets 4", L"[a-z][a-z]* *: *[1-9][0-9]*", L"k: 6000", L"k: 6000");
    do_test(L"More sets 5", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key: 1", L"key: 1");
    do_test(L"More sets 6", L"[1-9][0-9]*", L"6000", L"6000");

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
