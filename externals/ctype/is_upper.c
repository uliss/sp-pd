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

typedef struct is_upper_ {
    t_object xobj;
    t_outlet * outlet;
} t_is_upper;

#define PREFIX "[ctype:is_upper] "

t_class * is_upper_class;

void * is_upper_new(t_symbol *s, int argc, t_atom * argv)
{
    t_is_upper * res = (t_is_upper*) pd_new(is_upper_class);
    res->outlet = outlet_new(&res->xobj, &s_symbol);
    return (void*) res;
}

static void is_upper_symbol(t_is_upper * x, t_symbol * s)
{   
    int c = s->s_name[0];
    outlet_float(x->outlet, isupper(c) ? 1 : 0); 
}

void is_upper_setup()
{
   is_upper_class = class_new(gensym("is_upper"),
                              (t_newmethod) is_upper_new,
                               0,
                               sizeof(t_is_upper),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(is_upper_class, is_upper_symbol);
    class_sethelpsymbol(is_upper_class, gensym("is_upper-help.pd"));
}


