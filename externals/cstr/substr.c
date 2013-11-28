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

typedef struct substr_ {
    t_object xobj;
    t_symbol * substr;
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
    res->substr = NULL;
    
    switch(argc){
        default:
        case 2:
            res->length = atom_getint(argv + 1);
            // post("settings substr length to %d", res->length);
        case 1:
            res->start = atom_getint(argv);
            // post("settings substr start to %d", res->start);
        case 0:
        break;
    }

    res->outlet = outlet_new(&res->xobj, &s_symbol);
    res->start_inlet = floatinlet_new(&res->xobj, &res->start);
    res->length_inlet = floatinlet_new(&res->xobj, &res->length);

    return (void*) res;
}

static void substr_symbol(t_substr * x, t_symbol * f)
{
    const char * src = f->s_name;
    size_t src_len = strlen(src);
     
    if(src_len == 0) {
        error(PREFIX "empty string given");
        return;
    }
    
    size_t src_pos = 0;
    
    if(x->start >= 0) {
        src_pos = (size_t) x->start;
    }
    else {
        if(((long) src_len + (long) x->start) < 0)
            src_pos = 0;
        else
            src_pos = src_len + (long) x->start;
    }
             
    if(src_pos >= src_len) {
        error(PREFIX "invalid substring start position - %zd", src_pos);
        return;
    }
        
    size_t dest_len = 0;
    if(x->length == 0) {
        return; // length == 0, no copy
    }
        
    if(x->length > 0) {
        dest_len = (size_t) x->length;
        if(dest_len + src_pos >= src_len) { // to many characters
            dest_len = src_len - src_pos;
        }
    }
    else { // copy until end
        dest_len = src_len - src_pos;
    }
        
    char buffer[dest_len + 1];
    strncpy(buffer, &src[src_pos], dest_len);
    buffer[dest_len] = '\0';
    x->substr = gensym(buffer);
    outlet_symbol(x->outlet, x->substr);    
}

static void substr_bang(t_substr * x)
{
    if(x->substr != NULL)
        outlet_symbol(x->outlet, x->substr);
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
    class_addbang(substr_class, substr_bang);
    class_addanything(substr_class, substr_any);
    class_sethelpsymbol(substr_class, gensym("substr-help.pd"));
}


