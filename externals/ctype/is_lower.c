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

typedef struct is_lower_ {
    t_object xobj;
    t_outlet * outlet;
} t_is_lower;

#define PREFIX "[ctype:is_lower] "

t_class * is_lower_class;

void * is_lower_new(t_symbol *s, int argc, t_atom * argv)
{
    t_is_lower * res = (t_is_lower*) pd_new(is_lower_class);
    res->outlet = outlet_new(&res->xobj, &s_symbol);
    return (void*) res;
}

static void is_lower_symbol(t_is_lower * x, t_symbol * s)
{   
    int c = s->s_name[0];
    outlet_float(x->outlet, islower(c) ? 1 : 0); 
}

void is_lower_setup()
{
   is_lower_class = class_new(gensym("is_lower"),
                              (t_newmethod) is_lower_new,
                               0,
                               sizeof(t_is_lower),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(is_lower_class, is_lower_symbol);
    class_sethelpsymbol(is_lower_class, gensym("is_lower-help.pd"));
}


