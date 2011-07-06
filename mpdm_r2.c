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

    while (*rx && text[*o]) {

        if (rx[1] == L'?') {
            c = *rx;
            rx += 2;

            if (rx_test_char(c, text[*o]))
                (*o)++;
        }
        else
        if (rx[1] == L'*') {
            int o2;
            c = *rx;
            rx += 2;

            for (;;) {
                o2 = *o;

                if (rx_match_here(rx, text, &o2)) {
                    rx = L"";
                    *o = o2;
                    break;
                }

                if (!text[*o] || !rx_test_char(c, text[*o]))
                    break;

                (*o)++;
            }
        }
        else
        if (rx_test_char(*rx, text[*o])) {
            rx++;
            (*o)++;
        }
        else
            break;
    }

    if (!text[*o] && *rx == L'$')
        rx++;

    return !*rx;
}

int rx_match(wchar_t *rx, wchar_t *text, int *begin, int *end)
{
    *begin = *end = 0;
    int r = 0;

    if (*rx == L'^')
        r = rx_match_here(rx + 1, text, end);
    else {
        for (;;) {
            *end = *begin;

            r = rx_match_here(rx, text, end);

            if (r || !text[*begin])
                break;

            (*begin)++;
        }
    }

    return r;
}

#define MATCH rx_match

int main(int argc, char *argv[])
{
    int r, o1, o2;

    r = MATCH(L"g.*text", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"string", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"^text", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"^this", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"has", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"text$", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"this", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"stri*ng", L"this string has text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"stri*ng", L"this string has string text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);
    r = MATCH(L"str.*ng", L"this string has string text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);

    return 0;
}
