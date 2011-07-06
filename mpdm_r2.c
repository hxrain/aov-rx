#include <wchar.h>

/** experimental regexes **/

/** based on http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html **/

static int matchstar(wchar_t c, wchar_t *regexp, wchar_t *text);

/* matchhere: search for regexp at beginning of text */
static int matchhere(wchar_t *regexp, wchar_t *text)
{
    int r = 0;

    if (!r && *regexp == L'$' && regexp[1] == L'\0')
        r = 1;

    if (regexp[0] == L'\0')
        r = 1;
    else
    if (regexp[1] == L'*')
        r = matchstar(regexp[0], regexp + 2, text);
    else
    if (regexp[0] == L'$' && regexp[1] == L'\0')
        r = (*text == L'\0');
    else
    if (*text != L'\0' && (regexp[0] == L'.' || regexp[0] == *text))
        r = matchhere(regexp + 1, text + 1);

    return r;
}

/* matchstar: search for c*regexp at beginning of text */
static int matchstar(wchar_t c, wchar_t *regexp, wchar_t *text)
{
    int r = 0;

    do {
        /* a * matches zero or more instances */
        r = matchhere(regexp, text);

    } while (!r && *text != L'\0' && (*text++ == c || c == L'.'));

    return r;
}

/* match: search for regexp anywhere in text */
int match(wchar_t *regexp, wchar_t *text)
{
    int r = 0;

    if (regexp[0] == L'^')
        r = matchhere(regexp + 1, text);
    else
    do {
        /* must look even if string is empty */
        r = matchhere(regexp, text);

    } while (!r && *text++ != L'\0');

    return r;
}


/** mine **/

static int rx_test_char(wchar_t r, wchar_t t)
{
    return r == L'.' || r == t;
}


static wchar_t *rx_match_here(wchar_t *regexp, wchar_t *text, int *o)
{
    wchar_t c;

    while (*regexp && text[*o]) {

        if (regexp[1] == L'?') {
            c = *regexp;
            regexp += 2;

            if (rx_test_char(c, text[*o]))
                (*o)++;
        }
        else
        if (regexp[1] == L'*') {
            int o2;
            wchar_t *r;
            c = *regexp;
            regexp += 2;

            for (;;) {
                o2 = *o;
                r = rx_match_here(regexp, text, &o2);

                if (!*r || !text[*o] || !rx_test_char(c, text[*o]))
                    break;

                (*o)++;
            }

            regexp = r;
        }
        else
        if (rx_test_char(*regexp, text[*o])) {
            regexp++;
            (*o)++;
        }
        else
            break;
    }

    if (!text[*o] && *regexp == L'$')
        regexp++;

    return regexp;
}

int rx_match(wchar_t *regexp, wchar_t *text, int *o1, int *o2)
{
    *o1 = *o2 = 0;
    wchar_t *r = NULL;

    if (*regexp == L'^')
        r = rx_match_here(regexp + 1, text, o2);
    else {
        for (;;) {
            *o2 = *o1;

            r = rx_match_here(regexp, text, o2);

            if (!*r || !text[*o1])
                break;

            (*o1)++;
        }
    }

    return r && !*r;
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
