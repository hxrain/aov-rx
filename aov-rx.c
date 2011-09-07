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
/** inspired by http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html **/

#include "aov-rx.h"

#define VERSION "0.3-dev"


/** code **/

int aov_rx_match_one(wchar_t *rx, wchar_t *text, int *ri, int *ti)
/* matches one subject */
{
    int found = 0;
    wchar_t c, t;

    c = rx[(*ri)++];
    t = text[*ti];

    if (c == L'[') {
        /* set */
        int cond = 1;

        if (rx[*ri] == L'^') {
            /* negative set */
            (*ri)++;
            cond = 0;
        }

        found = !cond;

        while ((c = rx[(*ri)++]) && c != L']') {
            wchar_t d = c;

            if (rx[*ri] == L'-') {
                /* range */
                (*ri)++;
                d = rx[(*ri)++];
            }

            if (t >= c && t <= d)
                found = cond;
        }

        if (found)
            (*ti)++;
    }
    else
    if (c == L'(') {
        /* sub-regex */
        found = aov_rx_match_here_sub(rx, text, ri, ti);
    }
    else {
        if (c == L'.')
            c = t;
        else
        if (c == L'\\') {
            /* escaped char */
            c = rx[(*ri)++];

            switch (c) {
            case L'n':  c = L'\n'; break;
            case L'r':  c = L'\r'; break;
            }
        }

        if (c == t) {
            (*ti)++;
            found = 1;
        }
    }

    return found;
}


int aov_rx_match_here_sub(wchar_t *rx, wchar_t *text, int *ri, int *ti)
{
    int done = 0;
    int to = *ti;

    while (!done) {
        aov_rx_match_here(rx, text, ri, ti);

        if (rx[*ri] == L'|' || rx[*ri] == L')') {
            int l = 1;

            /* move beyond the ), skipping others */
            while (l) {
                wchar_t c = rx[(*ri)++];

                if (c == L'(')
                    l++;
                if (c == L')')
                    l--;
            }

            done = 1;
        }
        else {
            wchar_t c;
            int l = 1;

            /* rewind */
            *ti = to;

            /* find next option */
            for (;;) {
                c = rx[(*ri)++];

                if (!c)
                    break;
                else
                if (c == L'(')
                    l++;
                else
                if (c == L')')
                    l--;

                if (l == 0) {
                    if (c == L'|' || c == ')')
                        break;
                }
            }

            if (c != L'|')
                done = -1;
        }
    }

    return done > 0;
}


int aov_rx_match_here(wchar_t *rx, wchar_t *text, int *ri, int *ti)
{
    int done = 0;

    while (!done) {
        if (rx[*ri] == L'\0') {
            /* out of rx? win */
            done = 1;
        }
        else
        if (text[*ti] == L'\0') {
            /* out of text */
            done = rx[*ri] == L'$' ? 1 : -1;
        }
        else {
            wchar_t p;
            int m, ro;

            /* try matching */
            ro = *ri;
            m = aov_rx_match_one(rx, text, ri, ti);

            /* take predicate */
            p = rx[*ri];

            if (p == L'?')
                (*ri)++;
            else
            if (m) {
                if (p == L'*') {
                    if (aov_rx_match_here(rx, text, ri, ti))
                        done = 1;
                    else
                        *ri = ro;
                }
            }
            else {
                /* not matched; any possibility? */
                if (p == L'*')
                    (*ri)++;
                else {
                    *ri = ro;
                    done = -1;
                }
            }
        }
    }

    return done > 0;
}


static int rx_test_char(wchar_t r, wchar_t t)
{
    return r == L'.' || r == t;
}


static int rx_match_here(wchar_t *rx, wchar_t *text, int *o)
{
    wchar_t c;
    int i = *o;
    int done = 0;

    while (!done) {

        if (*rx == L'\0') {
            /* out of rx: win */
            done = 1;
        }
        else
        if (text[i] == L'\0') {
            /* out of text */
            done = *rx == L'$' ? 1 : -1;
        }
        else
        if (*rx == L'\\') {
            /* escaped character */
            rx++;

            /* direct compare */
            if (*rx == text[i]) {
                rx++;
                i++;
            }
            else
                done = -1;
        }
        else
        if (rx[1] == L'?') {
            c = *rx;
            rx += 2;

            if (rx_test_char(c, text[i]))
                i++;
        }
        else
        if (rx[1] == L'*' || rx[1] == L'+') {
            c = *rx;
            rx += 2;

            /* if it is +, at least should happen one time */
            if (rx[-1] == L'+' && !rx_test_char(c, text[i]))
                done = -1;

            while (!done) {
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

            rx++;

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

    *o = i;

    return done > 0;
}

/**
 * rx_match - Matches a regular expression
 * @rx: the regular expression
 * @text: the text to be matched
 * @begin: starting point / start of match
 * @size: size of match
 *
 * Matches the @rx regular expression on the @text string. On input, the
 * @begin argument should contain the offset to start testing (to test
 * from the beginning, set it to 0). On output, and if the matching is
 * positive, the @begin and @size arguments shall contain the start and
 * size of the match, respectively.
 * 
 * Returns a positive number (> 0) if the match was effective.
 */
int rx_match(wchar_t *rx, wchar_t *text, int *begin, int *size)
{
    int r = 0;
    int end;

    end = *begin;

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
