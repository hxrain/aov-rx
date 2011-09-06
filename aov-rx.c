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
    int found = 1;
    int r = *ri;
    wchar_t c, t;

    c = rx[r++];
    t = text[*ti];

    if (c == L'[') {
        /* set */
        int cond = 1;

        if (rx[r] == L'^') {
            /* negative set */
            r++;
            cond = 0;
        }

        found = !cond;

        while ((c = rx[r++]) && c != L']') {
            wchar_t d = c;

            if (rx[r] == L'-') {
                /* range */
                d = rx[++r];
                r++;
            }

            if (t >= c && t <= d)
                found = cond;
        }
    }
    else
    if (c == L'(') {
        /* sub-regex */
        /* ... */
    }
    else
    if (c == L'\\') {
        /* escaped char */
        c = rx[r++];

        switch (c) {
        case L'n':  c = L'\n'; break;
        case L'r':  c = L'\r'; break;
        }

        if (c != t)
            found = 0;
    }
    else
    if (c != L'.' && c != t)
        found = 0;

    /* advance if found */
    if (found)
        (*ti)++;

    *ri = r;

    return found;
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
