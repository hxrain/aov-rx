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

#define VERSION "0.5-dev"


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
        int oc = c;
        wchar_t *r = frx;
        int min = 1, max = 1;

        /* nothing more to match? go */
        if (*r == L'\0' || *r == L')' || tx[c] == L'\0')
            break;

        /* end of alternate set? */
        if (*r == L'|') {
            frx = skip_to(r, L')');
            break;
        }

        if (*r == L'(') {
            /* sub-regexp */
            int ii = 0;

            r++;
            c += match_here(r, &tx[c], 0, &ii);
            r += ii;
        }
        else
        if (*r == L'[') {
            /* set */
            int f = 0;

            if (r[1] == L'^') {
                r = in_set(r + 2, tx[c], &f);
                f = !f;
            }
            else
                r = in_set(r + 1, tx[c], &f);

            if (f)
                c++;
        }
        else
        if (*r == L'.')
            c++;
        else {
            if (*r == L'\\')
                r++;

            if (*r == tx[c])
                c++;
        }

        /* parse quantifier */
        if (*r) {
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

        if (c > oc) {
            cnt++;

            /* upper limit not reached? try searching the same again one more time */
            if (!max || cnt < max) {
                if (min == 0) {
                    int ii = 0, nc;

                    if ((nc = match_here(r, tx, c, &ii)) > c) {
                        c = nc;
                        frx = r + ii;
                    }
                }
            }
            else {
                /* start with a different thing and keep moving */
                cnt = 0;
                frx = r;
            }
        }
        else {
            /* not matched; were previous matches enough? */
            if (cnt < min) {
                /* no; move to a possible alternative */
                c = 0;
                frx = skip_past(r, L'|');
            }
            else {
                /* yes; keep moving from further position seen */
                cnt = 0;
                frx = r;
            }
        }
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
