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

typedef struct str_prepend_ {
    t_object xobj;
    t_symbol * prepend;
    t_inlet * prepend_inlet;
    t_outlet * outlet;
} t_str_prepend;

#define PREFIX "[cstr:str_prepend] "

t_class * str_prepend_class;

void * str_prepend_new(t_symbol *s, int argc, t_atom * argv)
{
    t_str_prepend * res = (t_str_prepend*) pd_new(str_prepend_class);
    res->prepend = NULL;
    
    if(argc > 0)
        res->prepend = atom_getsymbol(argv);

    res->outlet = outlet_new(&res->xobj, &s_symbol);
    res->prepend_inlet = symbolinlet_new(&res->xobj, &res->prepend);

    return (void*) res;
}

static void str_prepend_symbol(t_str_prepend * x, t_symbol * f)
{   
    const char * str1 = f->s_name;
    const char * str2 = NULL;
    if(x->prepend)
        str2 = x->prepend->s_name;
    else
        str2 = "";
    
    const size_t str1_len = strlen(str1);
    const size_t str2_len = strlen(str2);
    const size_t BUFSIZE = str1_len + str2_len + 1;
    
    char * buffer = (char*) malloc(BUFSIZE);
    strncpy(buffer, str2, BUFSIZE);
    strncat(buffer, str1, BUFSIZE);
    outlet_symbol(x->outlet, gensym(buffer)); 
    free(buffer);   
}

void str_prepend_setup()
{
    str_prepend_class = class_new(gensym("str_prepend"),
                              (t_newmethod) str_prepend_new,
                               0,
                               sizeof(t_str_prepend),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(str_prepend_class, str_prepend_symbol);
    class_sethelpsymbol(str_prepend_class, gensym("str_prepend-help.pd"));
}


