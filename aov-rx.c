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

#define VERSION "0.3-dev"


/** code **/

int aov_rx_match_one(wchar_t *rx, wchar_t *text, int *ri, int *ti)
/*
    Matches one subject, which can be:
    * A set (within square brackets, possible negated with ^);
    * A sub-regex (within parens, with optional pipes for alternatives);
    * An escaped char;
    * A char.

    ri is left pointing to the predicate (or the next subject). If
    the match is positive, ti has moved forward.

    Returns non-zero if the match is positive.
*/
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


wchar_t aov_rx_skip_sub(wchar_t *rx, int *ri, int part)
/* skips a part or a full subregex */
{
    wchar_t c;

    while ((c = rx[(*ri)++])) {
        if (c == L'(')
            aov_rx_skip_sub(rx, ri, 0);
        else
        if (c == L')')
            break;
        else
        if (part && c == L'|')
            break;
    }

    return c;
}


int aov_rx_match_here_sub(wchar_t *rx, wchar_t *text, int *ri, int *ti)
/* matches a sub-regex, with ( | ) */
{
    int done = 0;
    int to = *ti;

    while (!done) {
        aov_rx_match_here(rx, text, ri, ti);

        if (rx[*ri] == L'|' || rx[*ri] == L')') {
            /* found: move to the end of the subrx */
            aov_rx_skip_sub(rx, ri, 0);

            done = 1;
        }
        else {
            /* not found: rewind to test again */
            *ti = to;

            /* move to next part, if there is any */
            if (aov_rx_skip_sub(rx, ri, 1) != L'|')
                done = -1;
        }
    }

    return done > 0;
}


int aov_rx_match_here(wchar_t *rx, wchar_t *text, int *ri, int *ti)
/* matches rx in &text[*ti] position */
{
    int done = 0;

    while (!done) {
        wchar_t c = rx[*ri];

        if (c == L'\0') {
            /* out of rx? win */
            done = 1;
        }
        else
        if (text[*ti] == L'\0') {
            /* out of text */
            done = c == L'$' ? 1 : -1;
        }
        else
        if (c == L'|' || c == L')') {
            /* never match */
            done = -1;
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


/**
 * aov_rx_match - Matches a regular expression
 * @rx: the regular expression
 * @text: the text to be matched
 * @begin: starting point (input) / start of match (output)
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
int aov_rx_match(wchar_t *rx, wchar_t *text, int *begin, int *size)
{
    int r, ti, ri;

    ri = 0;
    ti = *begin;

    if (*rx == L'^') {
        ri++;
        r = aov_rx_match_here(rx, text, &ri, &ti);
    }
    else {
        for (;;) {
            ti = *begin;

            r = aov_rx_match_here(rx, text, &ri, &ti);

            if (r || !text[ti])
                break;

            (*begin)++;
        }
    }

    *size = ti - *begin;

    return r;
}
