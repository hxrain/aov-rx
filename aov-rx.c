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


static wchar_t *parse_quantifier(wchar_t *rx, int lim[2])
{
    lim[0] = 1; lim[1] = 1;

    if (*rx) {
        rx++;

        switch (*rx) {
        case L'?': lim[0] = 0; lim[1] = 1; rx++; break;
        case L'*': lim[0] = 0; lim[1] = 0; rx++; break;
        case L'+': lim[0] = 1; lim[1] = 0; rx++; break;
        case L'{': /* .... */ break;
        }
    }

    return rx;
}


static wchar_t *skip_past(wchar_t *rx, wchar_t c);

static wchar_t *skip_to(wchar_t *rx, wchar_t c)
{
    rx++;

    while (*rx && *rx != c) {
        if (*rx == L'(')
            rx = skip_past(rx, L')');
        else
            rx++;
    }

    return rx;
}


static wchar_t *skip_past(wchar_t *rx, wchar_t c)
{
    rx = skip_to(rx, c);
    if (*rx) rx++;

    return rx;
}


static int match_here(wchar_t *rx, wchar_t *tx, int c, int *i)
{
    wchar_t *frx = rx;
    int cnt = 0;

    for (;;) {
        int l[2], oc = c;
        wchar_t *r = frx;

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

        r = parse_quantifier(r, l);

        if (c > oc) {
            cnt++;

            /* upper limit not reached? try searching the same again one more time */
            if (!l[1] || cnt < l[1]) {
                if (l[0] == 0) {
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
            if (cnt >= l[0]) {
                /* yes; keep moving from further position seen */
                cnt = 0;
                frx = r;
            }
            else {
                /* no; move to a possible alternative */
                c = 0;
                frx = skip_past(r, L'|');
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
