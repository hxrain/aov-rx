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


static wchar_t *rx_match_here(wchar_t *regexp, wchar_t *text)
{
    wchar_t c;

    while (*regexp && *text) {

//        wprintf(L"%s, %s\n", regexp, text);

        if (regexp[1] == L'?') {
            c = *regexp;
            regexp += 2;

            if (rx_test_char(c, *text))
                text++;
        }
        else
        if (regexp[1] == L'*') {
            c = *regexp;
            regexp += 2;

            do {
                regexp = rx_match_here(regexp, text);
            }
            while (*regexp && *text && rx_test_char(c, *text++));
        }
        else
        if (rx_test_char(*regexp, *text)) {
            regexp++;
            text++;
        }
        else
            break;
    }

    if (!*text && *regexp == L'$')
        regexp++;

    return regexp;
}

int rx_match(wchar_t *regexp, wchar_t *text, int *offset)
{
    *offset = 0;

    if (*regexp == L'^')
        regexp = rx_match_here(regexp + 1, text);
    else
    do {
        regexp = rx_match_here(regexp, &text[*offset]);
    } while (*regexp && text[(*offset)++]);

    return !*regexp;
}

#define MATCH rx_match

int main(int argc, char *argv[])
{
    int r, o;

    r = MATCH(L"^text", L"this string has text", &o);
    printf("res: %d, offset: %d\n", r, o);
    r = MATCH(L"^this", L"this string has text", &o);
    printf("res: %d, offset: %d\n", r, o);
    r = MATCH(L"g.*text", L"this string has text", &o);
    printf("res: %d, offset: %d\n", r, o);
    r = MATCH(L"has", L"this string has text", &o);
    printf("res: %d, offset: %d\n", r, o);
    r = MATCH(L"text$", L"this string has text", &o);
    printf("res: %d, offset: %d\n", r, o);

    return 0;
}
