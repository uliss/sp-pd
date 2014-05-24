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
#include <stdlib.h>
#include <m_pd.h>

typedef struct str_compare_ {
    t_object xobj;
    t_symbol * s2;
    t_inlet * compare_inlet;
    t_outlet * outlet;
} t_str_compare;

#define PREFIX "[cstr:str_compare] "

t_class * str_compare_class;

void * str_compare_new(t_symbol *s, int argc, t_atom * argv)
{
    t_str_compare * res = (t_str_compare*) pd_new(str_compare_class);
    res->s2 = NULL;
    
    if(argc > 0)
        res->s2 = atom_getsymbol(argv);

    res->outlet = outlet_new(&res->xobj, &s_symbol);
    res->compare_inlet = symbolinlet_new(&res->xobj, &res->s2);

    return (void*) res;
}

static void str_compare_symbol(t_str_compare * x, t_symbol * f)
{   
    const char * str1 = f->s_name;
    const char * str2 = NULL;
    if(x->s2)
        str2 = x->s2->s_name;
    else
        str2 = "";
    
    int r = strcmp(str1, str2);
    int res = 0;
    if(r > 0)
       res = 1;
    else if(r == 0)
       res = 0;
    else
       res = -1;

    outlet_float(x->outlet, res); 
}

static void str_compare_list(t_str_compare * x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol * s1;

    if(argc > 0)
        s1 = atom_getsymbol(&argv[0]);    
    else
        return;

    if(argc > 1)
        x->s2 = atom_getsymbol(&argv[1]);

    str_compare_symbol(x, s1);
}

static void str_compare_any(t_str_compare * x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol * s1;
    
    if(argc > 0)
        s1 = atom_getsymbol(&argv[0]);
    else
        return;

    if(argc > 1)
        x->s2 = atom_getsymbol(&argv[0]);
   
    str_compare_symbol(x, s1);
}

void str_compare_setup()
{
    str_compare_class = class_new(gensym("str_compare"),
                              (t_newmethod) str_compare_new,
                               0,
                               sizeof(t_str_compare),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(str_compare_class, str_compare_symbol);
    class_addlist(str_compare_class, str_compare_list);
    class_addanything(str_compare_class, str_compare_any);
    class_sethelpsymbol(str_compare_class, gensym("str_compare-help.pd"));
}


