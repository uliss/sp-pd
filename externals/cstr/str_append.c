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

typedef struct str_append_ {
    t_object xobj;
    t_symbol * append;
    t_inlet * append_inlet;
    t_outlet * outlet;
} t_str_append;

#define PREFIX "[cstr:str_append] "

t_class * str_append_class;

void * str_append_new(t_symbol *s, int argc, t_atom * argv)
{
    t_str_append * res = (t_str_append*) pd_new(str_append_class);
    res->append = NULL;
    
    if(argc > 0)
        res->append = atom_getsymbol(argv);

    res->outlet = outlet_new(&res->xobj, &s_symbol);
    res->append_inlet = symbolinlet_new(&res->xobj, &res->append);

    return (void*) res;
}

static void str_append_symbol(t_str_append * x, t_symbol * f)
{   
    const char * str1 = f->s_name;
    const char * str2 = NULL;
    if(x->append)
        str2 = x->append->s_name;
    else
        str2 = "";
    
    const size_t str1_len = strlen(str1);
    const size_t str2_len = strlen(str2);
    const size_t BUFSIZE = str1_len + str2_len + 1;
    
    char * buffer = (char*) malloc(BUFSIZE);
    strncpy(buffer, str1, BUFSIZE);
    strncat(buffer, str2, BUFSIZE);
    outlet_symbol(x->outlet, gensym(buffer)); 
    free(buffer);   
}

void str_append_setup()
{
    str_append_class = class_new(gensym("str_append"),
                              (t_newmethod) str_append_new,
                               0,
                               sizeof(t_str_append),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(str_append_class, str_append_symbol);
    class_sethelpsymbol(str_append_class, gensym("str_append-help.pd"));
}


