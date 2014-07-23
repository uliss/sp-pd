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

typedef struct str_split_ {
    t_object xobj;
    t_symbol * tokens;
    t_inlet * tokens_inlet;
    t_outlet * outlet;
} t_str_split;

#define PREFIX "[cstr:str_split] "

t_class * str_split_class;

void * str_split_new(t_symbol *s, int argc, t_atom * argv)
{
    t_str_split * res = (t_str_split*) pd_new(str_split_class);
    res->tokens = NULL;
    
    if(argc > 0)
        res->tokens = atom_getsymbol(argv);

    res->outlet = outlet_new(&res->xobj, &s_symbol);
    res->tokens_inlet = symbolinlet_new(&res->xobj, &res->tokens);

    return (void*) res;
}

static int str_num_tok(const char * str, const char * tok)
{
    const size_t len = strlen(str);
    char * str_copy = (char*) malloc(sizeof(char) * len + 1);
    strcpy(str_copy, str);

    char * pch = strtok(str_copy, tok);
    int num = 0;
    while(pch != NULL) {
        pch = strtok(NULL, tok);
        num++; 
    }
    free(str_copy);
    return num;
}

static void str_split_symbol(t_str_split * x, t_symbol * f)
{    
    const char * tok = NULL;
    if(x->tokens)
        tok = x->tokens->s_name;
    else
        tok = "";
    
    post("tokens: %s", tok);

    const size_t len = strlen(f->s_name);
    char * str = (char*) malloc(sizeof(char) * len + 1);
    strcpy(str, f->s_name);
    post("string to split: %s", str); 

    const int num = str_num_tok(str, tok);

    post("tokens num: %li", len);

    if(num == 0) {
        outlet_symbol(x->outlet, f);
        free(str);
        return;
    }
    
    char * pch = strtok(str, tok);
    t_atom * argv = (t_atom*) malloc(sizeof(t_atom) * num);
    t_atom * pargv = argv;
    while(pch != NULL) {
        SETSYMBOL(pargv, gensym(pch));
        post("token found: %s", pch);
        pargv++;
        pch = strtok (NULL, tok);
    }

    outlet_list(x->outlet, &s_list, num, argv); 
    free(argv);
    free(str);
}

void str_split_setup()
{
    str_split_class = class_new(gensym("str_split"),
                              (t_newmethod) str_split_new,
                               0,
                               sizeof(t_str_split),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(str_split_class, str_split_symbol);
    //class_addlist(str_compare_class, str_compare_list);
    //class_addanything(str_compare_class, str_compare_any);
    class_sethelpsymbol(str_split_class, gensym("str_split-help.pd"));
}


