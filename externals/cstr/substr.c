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

#include "str_common.h"
#include "utf8.h"

typedef struct substr_ {
    t_object xobj;
    t_float start;
    t_float length;
    t_inlet * start_inlet;
    t_inlet * length_inlet;
    t_outlet * outlet;
} t_substr;

#define PREFIX "[cstr:substr] "

void substr_setup();

t_class * substr_class;

void * substr_new(t_symbol *s, int argc, t_atom * argv)
{
    t_substr * res = (t_substr*) pd_new(substr_class);
    res->start = 0;
    res->length = -1;
    
    switch(argc){
        default:
        case 2:
            res->length = atom_getint(argv + 1);
        case 1:
            res->start = atom_getint(argv);
        case 0:
        break;
    }

    res->outlet = outlet_new(&res->xobj, &s_symbol);
    res->start_inlet = floatinlet_new(&res->xobj, &res->start);
    res->length_inlet = floatinlet_new(&res->xobj, &res->length);

    return (void*) res;
}

static long substr_length(long str_len, long pos, long substr_len)
{
    if(substr_len == 0) 
        return -1; // length == 0, no copy
    
    if(substr_len > 0) { // positive
        if(pos + substr_len > str_len) // to many characters
            return str_len - pos;
        else
            return substr_len;
    }
    else { // copy until end
        return str_len - pos;
    }
}

static void substr_utf8(t_substr * x, t_symbol * s)
{
    const char * src = s->s_name;
    const long src_len = utf8_strlen(src);
    if(src_len == -1) {
        error(PREFIX "invalid utf-8");
        return;
    }
    
    const long src_pos = str_position(src_len, x->start);
    if(src_pos == -1) {
        error(PREFIX "invalid substring start position - %d", (int) x->start);
        return;
    }
    
    size_t dest_len = substr_length(src_len, src_pos, x->length);
    if(dest_len == -1)
        return;
    
    char buffer[src_len + 10];
    memset(buffer, 0, sizeof(buffer));
    int rc = utf8_substr(src, src_pos, dest_len, buffer);
    if(rc == -1) {
        error(PREFIX "utf8 substring error: %ld, %ld", src_pos, dest_len);
        return;
    }
    
    outlet_symbol(x->outlet, gensym(buffer));  
}

static void substr_symbol(t_substr * x, t_symbol * f)
{
    const char * src = f->s_name;
    size_t src_len = strlen(src);
    
    const long src_pos = str_position(src_len, x->start);             
    if(src_pos == -1) {
        error(PREFIX "invalid substring start position - %d", (int) x->start);
        return;
    }
        
    size_t dest_len = substr_length(src_len, src_pos, x->length);
    if(dest_len == -1)
        return;
        
    char buffer[dest_len + 1];
    strncpy(buffer, &src[src_pos], dest_len);
    buffer[dest_len] = '\0';
    outlet_symbol(x->outlet, gensym(buffer));    
}

static void substr_any(t_substr * x, t_symbol * s, int argc, t_atom * argv)
{
    // list message
    if(strncmp(s->s_name, "list", 4) == 0 && argc > 0) { 
        t_symbol * arg1 = atom_gensym(argv); // get first list item and try to convert to symbol
        if(arg1 != NULL)
            substr_symbol(x, arg1);
        
        return;
    }
    
    // float
    if(strncmp(s->s_name, "float", 5) == 0) {
        error(PREFIX "invalid message type: float");
        return;
    }
    
    // bang
    if(strncmp(s->s_name, "bang", 4) == 0) {
        error(PREFIX "invalid message type: bang");
        return;
    }
    
    // any other message
    substr_symbol(x, s);
}

void substr_setup()
{
    substr_class = class_new(gensym("substr"),
                              (t_newmethod) substr_new,
                               0,
                               sizeof(t_substr),
                               CLASS_DEFAULT,
                               A_GIMME,
                               0);

    class_addsymbol(substr_class, substr_symbol);
    class_addanything(substr_class, substr_any);
    class_addmethod(substr_class, 
                    (t_method) substr_utf8, 
                    gensym("utf8"),
                    A_DEFSYMBOL, 
                    0);
    class_sethelpsymbol(substr_class, gensym("substr-help.pd"));
}


