/*

    aov-rx - Angel Ortega's regular expression library

    Angel Ortega <angel@triptico.com>

    Public domain.

*/

#include "aov-rx.h"

#define VERSION "0.52"


static wchar_t *in_set(wchar_t *rx, wchar_t c, int *found)
{
    if (*rx && *rx != L']') {
        if (rx[1] == L'-') {
            if (c >= *rx && c <= rx[2])
                *found = 1;

            rx = in_set(rx + 3, c, found);
        }
        else {
            if (*rx == c)
                *found = 1;

            rx = in_set(rx + 1, c, found);
        }
    }

    return rx;
}


static wchar_t *skip_past(wchar_t *rx, wchar_t c);

static wchar_t *skip_to(wchar_t *rx, wchar_t c)
{
    if (*rx) {
        rx++;

        while (*rx && *rx != L'$' && *rx != L')' && *rx != c) {
            if (*rx == L'(')
                rx = skip_past(rx, L')');
            else
                rx++;
        }
    }

    return rx;
}


static wchar_t *skip_past(wchar_t *rx, wchar_t c)
{
    rx = skip_to(rx, c);
    if (*rx == c) rx++;

    return rx;
}


static wchar_t *parse_int(wchar_t *rx, int *v)
{
    *v = 0;

    while (*rx >= L'0' && *rx <= L'9') {
        *v = (*v * 10) + (*rx - L'0');
        rx++;
    }

    return rx;
}


struct rxctl {
    wchar_t *rx;
    wchar_t *tx;
    int     m;
};

static wchar_t *match_here_r(wchar_t *rx, wchar_t *tx, size_t *size);

static void match_here(struct rxctl *r, int cnt)
{
    if (*r->rx != L'\0' && *r->rx != L'|' && *r->rx != L')') {
        size_t it = 0;
        int min = 1, max = 1;
        wchar_t *orx = r->rx;

        if (*r->rx == L'(') {
            r->rx = match_here_r(
                r->rx + 1, &r->tx[r->m], &it);      /* subregex */

            if (*r->rx == L'|')
                r->rx = skip_to(r->rx, L')');
        }
        else
        if (*r->rx == L'[') {
            int f = 0;

            if (r->rx[1] == L'^') {
                r->rx = in_set(r->rx + 2, r->tx[r->m], &f);
                f = !f;
            }
            else
                r->rx = in_set(r->rx + 1, r->tx[r->m], &f);

            if (f)
                it++;                               /* matched set */
        }
        else
        if (r->tx[r->m] == L'\0') {
            if (*r->rx == L'$')
                it++;                               /* matched $ */
        }
        else
        if (*r->rx == L'.')
            it++;                                   /* matched . */
        else
        if (*r->rx == L'\\' && *++r->rx == r->tx[r->m])
            it++;                                   /* matched escaped */
        else
        if (*r->rx == r->tx[r->m])
            it++;                                   /* exact match */

        if (*r->rx) {
            r->rx++;                                /* parse quantifier */

            switch (*r->rx) {
            case L'?':  min = 0; max = 1; r->rx++; break;
            case L'*':  min = 0; max = 0x7fffffff; r->rx++; break;
            case L'+':  min = 1; max = 0x7fffffff; r->rx++; break;
            case L'{':  r->rx = parse_int(r->rx + 1, &min);
                if (*r->rx == L',')
                    r->rx = parse_int(r->rx + 1, &max);
                else
                    max = min;

                r->rx++;
                break;
            }
        }

        if (min == 0) {
            size_t m;

            match_here_r(r->rx, &r->tx[r->m], &m);  /* min == 0 restart */

            if (m) {
                r->m += m;                          /* match from here */
                return;
            }
        }

        r->m += it;

        if (it > 0) {
            cnt++;

            if (cnt == max)
                match_here(r, 0);                   /* restart */
            else {
                r->rx = orx;
                match_here(r, cnt);                 /* try repetition */
            }
        }
        else {
            if (cnt >= min)
                match_here(r, 0);                   /* enough prev. cnt */
            else {
                r->m  = 0;
                r->rx = skip_past(r->rx, L'|');
                match_here(r, 0);                   /* nonmatch retry */
            }
        }
    }
}


static wchar_t *match_here_r(wchar_t *rx, wchar_t *tx, size_t *size)
{
    struct rxctl r;

    r.rx    = rx;
    r.tx    = tx;
    r.m     = 0;

    *size = 0;
    match_here(&r, 0);
    *size = r.m;

    return r.rx;
}


/**
 * aov_match - Matches a regular expression
 * @rx: the regular expression
 * @tx: the text to be matched
 * @size: a pointer to a size_t where the matching length is stored
 *
 * Matches the string @tx for the regular expression in @rx.
 * On output, the size_t pointer by @size will contain the number
 * of matched characters (with 0 meaning that no matching was
 * possible). If the end of string mark ($) is used in the regular
 * expression and a match is effective, the ending zero is included
 * in the match.
 *
 * Returns the address of the match.
 */
wchar_t *aov_match(wchar_t *rx, wchar_t *tx, size_t *size)
{
    if (*rx == L'^')
        match_here_r(rx + 1, tx, size);
    else
    do {
        match_here_r(rx, tx, size);
    } while (!*size && *tx++);

    return tx;
}
