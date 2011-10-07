#include <stdio.h>
#include <wchar.h>
#include "aov-rx.h"

/* total number of tests and oks */
int tests = 0;
int oks = 0;

/* failed tests messages */
char *failed_msgs[5000];
int i_failed_msgs = 0;


void _do_test(wchar_t *desc, wchar_t *rx, wchar_t *txt,
                int exp_i, wchar_t *exp_s, int src_line)
{
    int begin, size;

    wprintf(L"#%d %ls: ", tests, desc);

    tests++;

    begin = 0;
    if (aov_rx_match(rx, txt, &begin, &size) == exp_i) {
        /* match: test now the string */
        if (!exp_i || wcsncmp(txt + begin, exp_s, size) == 0) {
            oks++;

            wprintf(L"OK (line %d)\n", src_line);
        }
        else {
            wchar_t tmp[16000];

            wcsncpy(tmp, txt + begin, size);
            tmp[size] = L'\0';

            /* different expected strings */
            wprintf(L"*** Failed *** (line %d): ", src_line);
            wprintf(L"got [%ls], expected [%ls]\n", tmp, exp_s);
        }
    }
    else {
        wprintf(L"*** Failed *** (line %d): NOT MATCH", src_line);
        wprintf(L", expected [%ls]\n", exp_s);
    }
}


int test_summary(void)
{
    wprintf(L"\n*** Total tests passed: %d/%d\n", oks, tests);

    if (oks == tests)
        wprintf(L"*** ALL TESTS PASSED\n");
    else {
        int n;

        wprintf(L"*** %d %s\n", tests - oks, "TESTS ---FAILED---");

/*        printf("\nFailed tests:\n\n");
        for (n = 0; n < i_failed_msgs; n++)
            printf("%s", failed_msgs[n]);*/
    }

    return oks == tests ? 0 : 1;
}


#define do_test(d, r, t, i, s) _do_test(d, r, t, i, s, __LINE__)


int main(int argc, char *argv[])
{
    int r, o1, o2;

    /* ^ */
    do_test(L"Non-matching ^", L"^text", L"this string has text", 0, L"");
    do_test(L"Matching ^", L"^this", L"this string has text", 1, L"this");

    /* basic */
    do_test(L"Basic string", L"string", L"this string has text", 1, L"string");
    do_test(L"Dots", L"h.la", L"hola", 1, L"hola");

    /* * */
    do_test(L".* 1", L"g.*text", L"this string has text", 1, L"g has text");
    do_test(L"i* 1", L"stri*ng", L"this string has text", 1, L"string");
    do_test(L".* is greedy", L"str.*ng", L"this string has string text", 1, L"string has string");
    do_test(L"More than 1 .*", L"str.*ng.*x", L"this string has string text", 1, L"string has string tex");
    do_test(L"* match to the end", L"one *world", L"one world", 1, L"one world");
    do_test(L"More *", L"a*bc", L"aaabc", 1, L"aaabc");

    /* ? */
    do_test(L"? 1", L"https?://", L"http://triptico.com", 1, L"http://");
    do_test(L"? 2", L"https?://", L"https://triptico.com", 1, L"https://");
    do_test(L"? 3", L"hos?la", L"hocla", 0, L"");
    do_test(L"? 4", L"hos?la", L"hola", 1, L"hola");
    do_test(L"? 5", L"hos?la", L"hosla", 1, L"hosla");

    /* $ */
    do_test(L"Matching $ 1", L"text$", L"this string has text", 1, L"text");
    do_test(L"Matching $ 2", L"text$", L"this string has text alone", 0, L"");

    /* parens */
    do_test(L"Paren 1", L"(http|ftp)://", L"http://triptico.com", 1, L"http://");
    do_test(L"Paren 2", L"(http|ftp)://", L"ftp://triptico.com", 1, L"ftp://");
    do_test(L"Paren 3", L"(http|ftp)s?://", L"http://triptico.com", 1, L"http://");
    do_test(L"Paren 4", L"(http|ftp)s?://", L"ftp://triptico.com", 1, L"ftp://");
    do_test(L"Paren 5", L"(http|ftp)s?://", L"ftps://triptico.com", 1, L"ftps://");
    do_test(L"Paren 6", L"(gopher|http|ftp)s?://", L"http://triptico.com", 1, L"http://");
    do_test(L"Paren 7", L".(es|com)$", L"http://triptico.com", 1, L".com");
    do_test(L"Paren 8", L".(es|com)$", L"http://triptico.es", 1, L".es");
    do_test(L"Paren 9", L".(es|com)$", L"http://triptico.org", 0, L"");

    /* + */
    do_test(L"+ 0 (really *)", L"one *world", L"oneworld is enough", 1, L"oneworld");
    do_test(L"+ 1", L"one +world", L"oneworld", 0, L"");
    do_test(L"+ 2", L"one +world", L"one world", 1, L"one world");
    do_test(L"+ 3", L"one +world", L"one    world", 1, L"one    world");
    do_test(L"+ 4", L"one +world", L"oneworld is enough", 0, L"");
    do_test(L"+ 5", L"one +world", L"one world is enough", 1, L"one world");
    do_test(L"+ 6", L"one +world", L"one    world is enough", 1, L"one    world");
    do_test(L"+ 7", L"one +world", L"I say oneworld is enough", 0, L"");
    do_test(L"+ 8", L"one +world", L"I say one world is enough", 1, L"one world");
    do_test(L"+ 9", L"one +world", L"I say one    world is enough", 1, L"one    world");

    /* escaped chars */
    do_test(L"esc 0 (really ?)", L"ready?", L"ready!", 1, L"ready!");
    do_test(L"esc 1", L"ready\\?", L"ready!", 0, L"");
    do_test(L"esc 2", L"ready\\?", L"ready?", 1, L"ready?");
    do_test(L"esc 3", L"triptico.com", L"tripticoxcom", 1, L"tripticoxcom");
    do_test(L"esc 4", L"triptico\\.com", L"tripticoxcom", 0, L"");
    do_test(L"esc 5", L"triptico\\.com", L"triptico.com", 1, L"triptico.com");
    do_test(L"esc 6", L"\\n", L"string without newlines", 0, L"");
    do_test(L"esc 7", L"\\n", L"I'm\nbroken", 1, L"\n");

    /* square bracket sets */
    do_test(L"[] 0", L"[^a-c]", L"z", 1, L"z");
    do_test(L"[] 1", L"[^a-cdzx]", L"z", 0, L"");
    do_test(L"[] 2", L"[a-c]", L"z", 0, L"");
    do_test(L"[] 3", L"[a-cdzx]", L"z", 1, L"z");
    do_test(L"[] 4", L"[a-c]", L"b", 1, L"b");
    do_test(L"[] 5", L"[abc]", L"b", 1, L"b");
    do_test(L"[] 6", L"[abc]", L"d", 0, L"");

    do_test(L"[] and * 0", L"[a-z][a-z]*", L"1234 string 456", 1, L"string");
    do_test(L"[] and * 1", L"[a-z][a-z]*:", L"1234 string key: value 456", 1, L"key:");
    do_test(L"[] and * 2", L"[a-c]*de", L"abcde", 1, L"abcde");

    /* alternate strings */
    do_test(L"Alt strings 0", L"(abc|def)1", L"try abf1 now", 0, L"");
    do_test(L"Alt strings 1", L"(abc|def)1", L"try def1 now", 1, L"def1");
    do_test(L"Alt strings 2", L"(abc|def)1", L"try abc1 now", 1, L"abc1");

    /* substrings */
    do_test(L"Substrs and * 0", L"Rem(ark)* comment", L"Rem comment", 1, L"Rem comment");
    do_test(L"Substrs and * 1", L"Rem(ark)* comment", L"Remarkark comment", 1, L"Remarkark comment");
    do_test(L"Substrs and * 2", L"Rem(ark)* comment", L"Remark comment", 1, L"Remark comment");
    do_test(L"Substrs and * 3", L"Rem(ark)* comment", L"<!-- Rem comment -->", 1, L"Rem comment");
    do_test(L"Substrs and ? 0", L"Rem(ark)? comment", L"Rem comment", 1, L"Rem comment");
    do_test(L"Substrs and ? 1", L"Rem(ark)? comment", L"Remark comment", 1, L"Remark comment");

    do_test(L"More sets 0", L"'[^']*'", L"I have here a 'string' between quotes", 1, L"'string'");
    do_test(L"More sets 1", L"'[^']*'", L"I have here a '' between quotes", 1, L"''");
    do_test(L"More sets 2", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key:15 # comment", 1, L"key:15");
    do_test(L"More sets 3", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key : 123456 # comment", 1, L"key : 123456");
    do_test(L"More sets 4", L"[a-z][a-z]* *: *[1-9][0-9]*", L"k: 6000", 1, L"k: 6000");
    do_test(L"More sets 5", L"[a-z][a-z]* *: *[1-9][0-9]*", L"key: 1", 1, L"key: 1");

    do_test(L"Brace matches 1 (like ?)", L"https{0,1}://", L"http://triptico.com", 1, L"http://");
    do_test(L"Brace matches 2 (like ?)", L"https{0,1}://", L"https://triptico.com", 1, L"https://");
    do_test(L"Brace matches 3 (like *)", L"a{0,}bc", L"aaabc", 1, L"aaabc");
    do_test(L"Brace matches 4 (like +)", L"one {1,}world", L"oneworld", 0, L"");
    do_test(L"Brace matches 5 (like +)", L"one {1,}world", L"one world", 1, L"one world");
    do_test(L"Brace matches 6 (like +)", L"one {1,}world", L"one    world", 1, L"one    world");

    do_test(L"Brace matches 11", L"a.{0,5}c", L"abc", 1, L"abc");
    do_test(L"Brace matches 12", L"a.{0,5}c", L"abcdec", 1, L"abcdec");
    do_test(L"Brace matches 13", L"a.{0,5}c", L"abcdecfghic", 0, L"");

    return test_summary();
}
