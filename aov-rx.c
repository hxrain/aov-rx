/*

    aov-rx - Angel Ortega's regular expression library

    Copyright (C) 2011 Angel Ortega <angel@triptico.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    http://triptico.com

*/

#include "aov-rx.h"

#define VERSION "0.5.0-dev"


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


static int match_here(wchar_t *rx, wchar_t *tx, int c, int *i)
{
    wchar_t *frx = rx;
    int cnt = 0;

    for (;;) {
        int a = 0;
        wchar_t *r = frx;
        wchar_t *t = &tx[c];
        int min = 1, max = 1;

        if (*r == L'|') {                   /* end of alternate set */
            frx = skip_to(r, L')');
            break;
        }

        if (*r == L'\0' || *r == L')')      /* end of match */
            break;

        if (*t == L'\0') {                  /* out of text */
            if (*r != L'$')
                c = 0;

            break;
        }

        if (*r == L'(') {                   /* sub-regexp */
            int ii = 0;

            r++;
            a = match_here(r, t, 0, &ii);
            r += ii;
        }
        else
        if (*r == L'[') {                   /* set */
            int f = 0;

            if (r[1] == L'^') {
                r = in_set(r + 2, *t, &f);
                f = !f;
            }
            else
                r = in_set(r + 1, *t, &f);

            if (f)
                a++;
        }
        else
        if (*r == L'.')                     /* any char */
            a++;
        else
        if (*r == L'\\' && *++r == *t)      /* escaped char */
            a++;
        else
        if (*r == *t)                       /* exact char */
            a++;

        if (*r) {                           /* parse quantifier */
            r++;

            switch (*r) {
            case L'?': min = 0; max = 1; r++; break;
            case L'*': min = 0; max = 0; r++; break;
            case L'+': min = 1; max = 0; r++; break;
            case L'{':
                r = parse_int(r + 1, &min);
                if (*r == L',')
                    r = parse_int(r + 1, &max);
                else
                    max = min;

                r++;
                break;
            }
        }

        if (a) {
            c += a;
            cnt++;

            /* upper limit not reached? try searching the same again one more time */
            if (!max || cnt < max) {
                if (min == 0) {
                    int ii = 0;

                    if ((a = match_here(r, &tx[c], 0, &ii))) {
                        c += a;
                        frx = r + ii;
                    }
                }

                continue;
            }
        }
        else {
            /* not matched; were previous matches enough? */
            if (cnt < min) {
                /* no; move to a possible alternative */
                c = 0;
                frx = skip_past(r, L'|');

                continue;
            }
        }

        cnt = 0;
        frx = r;
    }

    *i = frx - rx;

    return c;
}


wchar_t *aov_match(wchar_t *rx, wchar_t *tx, int *size)
{
    int i;

    *size = 0;

    if (*rx == L'^')
        *size = match_here(rx + 1, tx, 0, &i);
    else {
        while (*tx) {
            if ((*size = match_here(rx, tx, 0, &i)))
                break;

            tx++;
        }
    }

    return tx;
}


/** 0.5.1 **/

struct rxctl {
    wchar_t *rx;
    wchar_t *tx;
    int     m;
};

void match_05_here(struct rxctl *r, int cnt)
{
    if (*r->rx != L'\0' && *r->rx != L'|' && *r->rx != L')' && r->tx[r->m]) {
        int it = 0;
        int min, max;
        wchar_t *orx = r->rx;

        if (*r->rx == L'(') {
            struct rxctl sr;

            sr.rx   = r->rx + 1;
            sr.tx   = &r->tx[r->m];
            sr.m    = 0;

            match_05_here(&sr, 0);

            r->rx   = sr.rx;
            it      = sr.m;

            if (*r->rx == L'|')
                r->rx = skip_to(r->rx, L')');
        }
        else
        if (
            (*r->rx == L'.') ||
            (*r->rx == L'$' && r->tx[r->m] == L'\0') ||
            (*r->rx == r->tx[r->m])
        )
            it++;

        if (*r->rx) {
            r->rx++;

            switch (*r->rx) {
            case L'?':  min = 0; max = 1; r->rx++; break;
            case L'*':  min = 0; max = 0x7fffffff; r->rx++; break;
            case L'+':  min = 1; max = 0x7fffffff; r->rx++; break;
            default:    min = 1; max = 1; break;
            }
        }

        r->m += it;

        if (it > 0) {
            cnt++;

            if (cnt == max)
                match_05_here(r, 0);
            else {
                r->rx = orx;
                match_05_here(r, cnt);
            }
        }
        else {
            if (cnt >= min)
                match_05_here(r, 0);
            else {
                r->m  = 0;
                r->rx = skip_past(r->rx, L'|');
                match_05_here(r, 0);
            }
        }
    }
}


wchar_t *aov_05_match(wchar_t *rx, wchar_t *tx, int *size)
{
    struct rxctl r;

    r.rx    = rx;
    r.tx    = tx;
    r.m     = 0;

    if (*rx == L'^') {
        r.rx++;
        match_05_here(&r, 0);
    }
    else {
        while (*tx) {
            match_05_here(&r, 0);

            if (r.m)
                break;

            r.tx++;
        }
    }

    *size = r.m;

    return r.tx;
}
