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

#include "utf8.h"
#include "str_common.h"

typedef struct str_at_ {
    t_object xobj;
    t_float pos;
    t_inlet * pos_inlet;
    t_outlet * outlet;
} t_str_at;

#define PREFIX "[cstr:str_at] "

void str_at_setup();

t_class * str_at_class;

void * str_at_new(t_symbol *s, int argc, t_atom * argv)
{
    t_str_at * res = (t_str_at*) pd_new(str_at_class);
    res->pos = 0;
    
    if(argc > 0)
        res->pos = atom_getint(argv);

    res->outlet = outlet_new(&res->xobj, &s_symbol);
    res->pos_inlet = floatinlet_new(&res->xobj, &res->pos);

    return (void*) res;
}

static void str_at_symbol(t_str_at * x, t_symbol * f)
{
    const char * str = f->s_name;
    const long pos = str_position(strlen(str), x->pos);
             
    if(pos == -1) {
        error(PREFIX "invalid substring start position: %d", (int) x->pos);
        return;
    }
     
    char buffer[2];
    buffer[0] = str[pos];
    buffer[1] = '\0';
    outlet_symbol(x->outlet, gensym(buffer));    
}

static void str_at_utf8(t_str_at * x, t_symbol * s)
{
    const char * str = s->s_name;
    const long pos = str_position(utf8_strlen(str), x->pos);
    
    if(pos == -1) {
        error(PREFIX "invalid substring start position: %d", (int) x->pos);
        return;
    }
    
    char buffer[5] = {};
    int res = utf8_char_at(str, pos, buffer);
    if(res != 0) {
        error(PREFIX "invalid utf-8 char");
        return;
    }
    
    outlet_symbol(x->outlet, gensym(buffer));
}

void str_at_setup()
{
    str_at_class = class_new(gensym("str_at"),
                              (t_newmethod) str_at_new,
                               0,
                               sizeof(t_str_at),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(str_at_class, str_at_symbol);
    class_addmethod(str_at_class, 
                    (t_method) str_at_utf8, 
                    gensym("utf8"),
                    A_DEFSYMBOL, 
                    0);
    class_sethelpsymbol(str_at_class, gensym("str_at-help.pd"));
}


