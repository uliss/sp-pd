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

typedef struct is_digit_ {
    t_object xobj;
    t_outlet * outlet;
} t_is_lower;

#define PREFIX "[ctype:is_digit] "

t_class * is_digit_class;

void * is_digit_new(t_symbol *s, int argc, t_atom * argv)
{
    t_is_lower * res = (t_is_lower*) pd_new(is_digit_class);
    res->outlet = outlet_new(&res->xobj, &s_symbol);
    return (void*) res;
}

static void is_digit_symbol(t_is_lower * x, t_symbol * s)
{   
    post(PREFIX "symbol: %s", s->s_name);
    
    int c = s->s_name[0];
    outlet_float(x->outlet, isdigit(c) ? 1 : 0); 
}

static void is_digit_float(t_is_lower * x, t_float f)
{   
    post(PREFIX "%d", f);
    outlet_float(x->outlet, 1); 
}

void is_digit_setup()
{
   is_digit_class = class_new(gensym("is_digit"),
                              (t_newmethod) is_digit_new,
                               0,
                               sizeof(t_is_lower),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(is_digit_class, is_digit_symbol);
    class_addfloat(is_digit_class, is_digit_float);
    class_sethelpsymbol(is_digit_class, gensym("is_digit-help.pd"));
}


