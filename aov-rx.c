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

#define VERSION "0.4-dev"


/** prototypes **/

int aov_rx_match_here_sub(wchar_t *rx, wchar_t *text, int *ri, int *ti);
int aov_rx_match_here(wchar_t *rx, wchar_t *text, int *ri, int *ti);


/** code **/

int aov_rx_match_one(wchar_t *rx, wchar_t *text, int *ri, int *ti)
/*
    Matches one subject, which can be:
    * A set (within square brackets, possible negated with ^);
    * A sub-regex (within parens, with optional pipes for alternatives);
    * An escaped char;
    * A char.

    ri is left pointing to the optional quantifier or the next subject.
    If the match has been positive, ti has moved forward.

    Returns non-zero if the match is positive.
*/
{
    int found = 0;
    wchar_t c, t;

    c = rx[(*ri)++];
    t = text[*ti];

    if (t == L'\0') {
        found = 0;
    }
    else
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


#define QUANT_NO_MAX 0x7fffffff

static void parse_quant(wchar_t *rx, int *ri, int *q_min, int *q_max)
/* parses a possible quantifier at &regex[*ri] */
{
    wchar_t c;
    int r, mi, ma;

    /* default quantifier */
    mi = ma = 1;

    r = *ri;

    c = rx[r];

    if (c == L'?') {
        mi = 0;
        ma = 1;
        r++;
    }
    else
    if (c == L'*') {
        mi = 0;
        ma = QUANT_NO_MAX;
        r++;
    }
    else
    if (c == L'+') {
        mi = 1;
        ma = QUANT_NO_MAX;
        r++;
    }
    else
    if (c == L'{') {
        /* range in curly braces */
        mi = ma = 0;

        /* before the comma */
        while ((c = rx[++r]) >= L'0' && c <= L'9')
            mi = (mi * 10) + (c - L'0');

        if (c == L',') {
            /* after the comma */
            while ((c = rx[++r]) >= L'0' && c <= L'9')
                ma = (ma * 10) + (c - L'0');
        }

        /* skip final curly bracket */
        if (c == L'}')
            r++;

        /* fix no max */
        if (ma == 0)
            ma = QUANT_NO_MAX;
    }

    *q_min  = mi;
    *q_max  = ma;
    *ri     = r;
}


int aov_rx_match_quant(wchar_t *rx, wchar_t *text, int *ri, int *ti)
/* matches a subject with its quantifier */
{
    int q_min, q, q_max;
    int ro, rn;
    int ret = 0;

    /* pick current position */
    ro = *ri;

    /* first match */
    q = aov_rx_match_one(rx, text, ri, ti);

    /* parse the (optional) quantifier that follows */
    parse_quant(rx, ri, &q_min, &q_max);

    /* the regex continues here */
    rn = *ri;

    while (q >= q_min && q < q_max) {
        int tt;

        /* if the regex matches from here, break the count */
        tt = *ti;
        *ri = rn;

        if (aov_rx_match_here(rx, text, ri, &tt))
            break;

        *ri = ro;

        /* stop counting if no more matches */
        if (!aov_rx_match_one(rx, text, ri, ti))
            break;

        q++;
    }

    /* still in range? ok for now */
    if (q >= q_min && q <= q_max) {
        ret = 1;
        *ri = rn;
    }

    return ret;
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
        else
        if (!aov_rx_match_quant(rx, text, ri, ti))
            done = -1;
    }

    return done > 0;
}


/** 0.5.x **/


wchar_t *in_set(wchar_t *rx, wchar_t *tx, int *found)
{
    if (*rx != L'\0' && *rx != L']') {
        if (rx[1] == L'-') {
            if (*tx >= *rx && *tx <= rx[2])
                *found = 1;

            rx = in_set(rx + 3, tx, found);
        }
        else {
            if (*rx == *tx)
                *found = 1;

            rx = in_set(rx + 1, tx, found);
        }
    }

    return rx;
}


int match_set(wchar_t **prx, wchar_t *tx)
{
    int found = 0;
    wchar_t *rx = *prx;

    if (*rx == L'[') {
        if (rx[1] == L'^') {
            rx = in_set(rx + 2, tx, &found);
            found = !found;
        }
        else
            rx = in_set(rx + 1, tx, &found);

        *prx = rx;
    }

    return found;
}


wchar_t *parse_quantif(wchar_t *rx, int *limit, int m)
{
    int lim[2] = { 1, 1 };

    rx++;

    switch (*rx) {
    case L'?': lim[0] = 0; lim[1] = 1; rx++; break;
    case L'*': lim[0] = 0; lim[1] = 0; rx++; break;
    case L'+': lim[0] = 1; lim[1] = 0; rx++; break;
    case L'{': /* .... */ break;
    }

    *limit = lim[m];

    return rx;
}


wchar_t *match_one(wchar_t **prx, wchar_t *tx, int *limit)
{
    wchar_t *rx = *prx;
    wchar_t *ntx = NULL;

    if (*rx == L'(') {
        /* it's a subregex */
    }
    else
    if (
        match_set(&rx, tx) ||
        (*rx == *tx) ||
        (*rx == L'.' && *tx) ||
        (*rx == L'\\' && *(++rx) == *tx)
    )
        ntx = tx + 1;

    rx = parse_quantif(rx, limit, ntx != NULL);

    if (ntx == NULL)
        *prx = rx;

    return ntx;
}


wchar_t *match_here(wchar_t *rx, wchar_t *tx);

wchar_t *match_here_cnt(wchar_t *rx, wchar_t *tx, int cnt)
{
    wchar_t *ntx;
    int limit;

    if ((ntx = match_one(&rx, tx, &limit)) == NULL) {
        if (cnt >= limit)
            tx = match_here(rx, tx);
        else
            tx = NULL;
    }
    else {
        if (!limit || cnt < limit)
            tx = match_here_cnt(rx, ntx, cnt + 1);
        else
            tx = match_here(rx, ntx);
    }

    return tx;
}


wchar_t *match_here(wchar_t *rx, wchar_t *tx)
{
    if (!(*rx == L'\0') && !(*rx == L'$' && *tx == L'\0'))
        tx = match_here_cnt(rx, tx, 0);

    return tx;
}


wchar_t *match(wchar_t *rx, wchar_t *tx, int *size)
{
    wchar_t *ntx = NULL;

    if (*rx == L'^')
        ntx = match_here(rx + 1, tx);
    else {
        while (*tx) {
            if ((ntx = match_here(rx, tx)))
                break;
            else
                tx++;
        }
    }

    *size = ntx ? ntx - tx : 0;

    return tx;
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
            ri = 0;
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
