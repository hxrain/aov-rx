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


/** 0.5.x **/


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


static wchar_t *parse_quantifier(wchar_t *rx, int *limit, int m)
{
    int lim[2] = { 1, 1 };

    switch (*rx) {
    case L'?': lim[0] = 0; lim[1] = 1; rx++; break;
    case L'*': lim[0] = 0; lim[1] = 0; rx++; break;
    case L'+': lim[0] = 1; lim[1] = 0; rx++; break;
    case L'{': /* .... */ break;
    }

    *limit = lim[m];

    return rx;
}


static wchar_t *skip_past(wchar_t *rx, wchar_t c)
{
    rx++;

    while (*rx && *rx != c) {
        if (*rx == L'(')
            rx = skip_past(rx, L')');

        rx++;
    }

    if (*rx) rx++;

    return rx;
}


static int match_here(wchar_t **rx, wchar_t *tx, int c);

static int match_one(wchar_t **prx, wchar_t *tx, int c, int *limit)
{
    wchar_t *rx = *prx;
    int oc = c;

    if (*rx == L'[') {
        int f = 0;

        if (rx[1] == L'^') {
            rx = in_set(rx + 2, tx[c], &f);
            f = !f;
        }
        else
            rx = in_set(rx + 1, tx[c], &f);

        if (f)
            c++;
    }
    if (*rx == L'.' && tx[c])
        c++;
    else {
        if (*rx == L'\\')
            rx++;

        if (*rx == tx[c])
            c++;
    }

    rx = parse_quantifier(rx + 1, limit, c > oc);

    if (c <= oc)
        *prx = rx;

    return c;
}


static int match_here_cnt(wchar_t **rx, wchar_t *tx, int c, int cnt)
{
    int l, oc = c;

    if ((c = match_one(rx, tx, c, &l)) > oc) {
        if (!l || cnt < l)
            c = match_here_cnt(rx, tx, c, cnt + 1);
        else
            c = match_here(rx, tx, c);
    }
    else {
        if (cnt >= l)
            c = match_here(rx, tx, c);
        else
//            c = 0;
        {
            wchar_t *t = skip_past(*rx, L'|');

            *rx = t;
            c = match_here(rx, tx, 0);
        }
    }

    return c;
}


static int match_here(wchar_t **rx, wchar_t *tx, int c)
{
    if (!(**rx == L'\0') && !(**rx == L'$' && tx[c] == L'\0')) {
        if (**rx == L'|') {
            wchar_t *t = skip_past(*rx, L')');

            *rx = t;
            c = match_here(rx, tx, c);
        }
        else
            c = match_here_cnt(rx, tx, c, 0);
    }

    return c;
}


wchar_t *match_s(wchar_t *rx, wchar_t *tx, int *size)
{
    *size = match_here(&rx, tx, 0);
    return tx;
}


wchar_t *match_l(wchar_t *rx, wchar_t *tx, int *size)
{
    if (*tx && !(*size = match_here(&rx, tx, 0)))
        tx = match_l(rx, tx + 1, size);

    return tx;
}


wchar_t *match(wchar_t *rx, wchar_t *tx, int *size)
{
    if (*rx == L'^')
        tx = match_s(rx + 1, tx, size);
    else
        tx = match_l(rx, tx, size);

    return tx;
}
