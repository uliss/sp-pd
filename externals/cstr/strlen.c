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

#include <string.h>
#include <m_pd.h>

typedef struct strlen_ {
    t_object xobj;
    t_outlet * outlet;
} t_strlen;

#define PREFIX "[cstr:strlen] "

void strlen_setup();

t_class * strlen_class;

void * strlen_new(t_symbol *s, int argc, t_atom * argv)
{
    t_strlen * res = (t_strlen*) pd_new(strlen_class);
    res->outlet = outlet_new(&res->xobj, &s_float);
    return (void*) res;
}

static void strlen_symbol(t_strlen * x, t_symbol * f)
{
    outlet_float(x->outlet, strlen(f->s_name)); 
}

void strlen_setup()
{
    strlen_class = class_new(gensym("strlen"),
                              (t_newmethod) strlen_new,
                               0,
                               sizeof(t_strlen),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(strlen_class, strlen_symbol);
    class_sethelpsymbol(strlen_class, gensym("strlen-help.pd"));
}


