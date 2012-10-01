/*

    aov-rx - Angel Ortega's regular expression library

    Copyright (C) 2011/2012 Angel Ortega <angel@triptico.com>

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

#define VERSION "0.5.1-dev"


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

static void match_here(struct rxctl *r, int cnt)
{
    if (*r->rx != L'\0' && *r->rx != L'|' && *r->rx != L')' /*&& r->tx[r->m]*/) {
        int it = 0;
        int min, max;
        wchar_t *orx = r->rx;

        if (*r->rx == L'(') {
            struct rxctl sr;

            sr.rx   = r->rx + 1;
            sr.tx   = &r->tx[r->m];
            sr.m    = 0;

            match_here(&sr, 0);

            r->rx   = sr.rx;
            it      = sr.m;

            if (*r->rx == L'|')
                r->rx = skip_to(r->rx, L')');
        }
        else
        if (r->tx[r->m] == L'\0') {
        }
        else
        if (*r->rx == L'[') {                   /* set */
            int f = 0;

            if (r->rx[1] == L'^') {
                r->rx = in_set(r->rx + 2, r->tx[r->m], &f);
                f = !f;
            }
            else
                r->rx = in_set(r->rx + 1, r->tx[r->m], &f);

            if (f)
                it++;
        }
        else
        if (
            (*r->rx == L'.') ||
            (*r->rx == L'$' && r->tx[r->m] == L'\0') ||
            (*r->rx == L'\\' && *++r->rx == r->tx[r->m]) ||
            (*r->rx == r->tx[r->m])
        )
            it++;

        if (*r->rx) {
            r->rx++;

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
            default:    min = 1; max = 1; break;
            }
        }

        r->m += it;

        if (it > 0) {
            cnt++;

            if (cnt == max)
                match_here(r, 0);
            else {
                r->rx = orx;
                match_here(r, cnt);
            }
        }
        else {
            if (cnt >= min)
                match_here(r, 0);
            else {
                r->m  = 0;
                r->rx = skip_past(r->rx, L'|');
                match_here(r, 0);
            }
        }
    }
}


wchar_t *aov_match(wchar_t *rx, wchar_t *tx, int *size)
{
    struct rxctl r;

    r.rx    = rx;
    r.tx    = tx;
    r.m     = 0;

    if (*rx == L'^') {
        r.rx++;
        match_here(&r, 0);
    }
    else {
        while (*r.tx) {
            r.rx = rx;

            match_here(&r, 0);

            if (r.m)
                break;

            r.tx++;
        }
    }

    *size = r.m;

    return r.tx;
}
