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

    while (*rx && text[i]) {

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
                    rx = L"";
                    i = o2;

                    break;
                }

                if (!text[i] || !rx_test_char(c, text[i]))
                    break;

                i++;
            }
        }
        else
        if (rx_test_char(*rx, text[i])) {
            rx++;
            i++;
        }
        else
            break;
    }

    if (!text[i] && *rx == L'$')
        rx++;

    *o = i;

    return !*rx;
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
    r = MATCH(L"str.*ng.*x", L"this string has string text", &o1, &o2);
    printf("res: %d, offset: %d, %d\n", r, o1, o2);

    return 0;
}
