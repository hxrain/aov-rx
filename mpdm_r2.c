#include <wchar.h>

/** experimental regexes **/

/** inspired by http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html **/

static int rx_test_char(wchar_t r, wchar_t t)
{
    return r == L'.' || r == t;
}


static int rx_match_here(wchar_t *rx, wchar_t *text, int *o)
{
    wchar_t c;
    int i = *o;
    int done = 0;

    while (!done && text[i]) {

        if (*rx == L'\0') {
            /* out of rx: win */
            done = 1;
        }
        else
        if (rx[1] == L'?') {
            c = *rx;
            rx += 2;

            if (rx_test_char(c, text[i]))
                i++;
        }
        else
        if (rx[1] == L'*') {
            c = *rx;
            rx += 2;

            for (;;) {
                int o2 = i;

                if (rx_match_here(rx, text, &o2)) {
                    done = 1;
                    i = o2;

                    break;
                }

                /* this test implies !text[i] */
                if (!rx_test_char(c, text[i]))
                    break;

                i++;
            }
        }
        else
        if (*rx == L'(') {
            /* paren matching: multiple strings */
            int p_done = 0;
            int i2;

            while (!p_done) {
                i2 = i;

                while (!p_done && rx_test_char(*rx, text[i2])) {
                    rx++;
                    i2++;

                    /* got to | or )? success */
                    if (*rx == L'|' || *rx == L')')
                        p_done = 1;
                }

                if (p_done) {
                    /* move past the ) */
                    while (*rx && *rx != L')')
                        rx++;

                    if (*rx == L')')
                        rx++;

                    i = i2;
                }
                else {
                    /* move past the | */
                    while (*rx && *rx != L'|' && *rx != L')')
                        rx++;

                    if (*rx == L'|')
                        rx++;
                    else {
                        /* all alternatives failed */
                        done = -1;
                        break;
                    }
                }
            }
        }
        else
        if (rx_test_char(*rx, text[i])) {
            rx++;
            i++;
        }
        else
            done = -1;
    }

    if (!done && !text[i] && *rx == L'$')
        done = 1;

    *o = i;

    return done > 0;
}

int rx_match(wchar_t *rx, wchar_t *text, int *begin, int *size)
{
    int r = 0;
    int end;

    *begin = end = 0;

    if (*rx == L'^')
        r = rx_match_here(rx + 1, text, &end);
    else {
        for (;;) {
            end = *begin;

            r = rx_match_here(rx, text, &end);

            if (r || !text[*begin])
                break;

            (*begin)++;
        }
    }

    *size = end - *begin;

    return r;
}



#ifdef STRESS

#include <stdio.h>

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

    begin = 0;

    tests++;

    wprintf(L"%ls: ", desc);

    if (rx_match(rx, txt, &begin, &size) == exp_i) {
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

    /* * */
    do_test(L".* 1", L"g.*text", L"this string has text", 1, L"g has text");
    do_test(L"i* 1", L"stri*ng", L"this string has text", 1, L"string");
    do_test(L"i* is non-greedy", L"stri*ng", L"this string has string text", 1, L"string");
    do_test(L".* is non-greedy", L"str.*ng", L"this string has string text", 1, L"string");
    do_test(L"More than 1 .*", L"str.*ng.*x", L"this string has string text", 1, L"string has string tex");

    /* ? */
    do_test(L"? 1", L"https?://", L"http://triptico.com", 1, L"http://");
    do_test(L"? 2", L"https?://", L"https://triptico.com", 1, L"https://");

    /* $ */
    do_test(L"Matching $", L"text$", L"this string has text", 1, L"text");

    return test_summary();
}
#endif /* STRESS */

