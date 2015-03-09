/***************************************************************************
 *   Copyright (C) 2013 by Serge Poltavski                                 *
 *   serge.poltavski@gmail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program. If not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <ctype.h>
#include <m_pd.h>

typedef struct is_punct_ {
    t_object xobj;
    t_outlet * outlet;
} t_is_punct;

#define PREFIX "[ctype:is_punct] "

t_class * is_punct_class;

void * is_punct_new(t_symbol *s, int argc, t_atom * argv)
{
    t_is_punct * res = (t_is_punct*) pd_new(is_punct_class);
    res->outlet = outlet_new(&res->xobj, &s_symbol);
    return (void*) res;
}

static void is_punct_symbol(t_is_punct * x, t_symbol * s)
{   
    int c = s->s_name[0];
    outlet_float(x->outlet, ispunct(c) ? 1 : 0); 
}

void is_punct_setup()
{
   is_punct_class = class_new(gensym("is_punct"),
                              (t_newmethod) is_punct_new,
                               0,
                               sizeof(t_is_punct),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(is_punct_class, is_punct_symbol);
    class_sethelpsymbol(is_punct_class, gensym("is_punct-help.pd"));
}


