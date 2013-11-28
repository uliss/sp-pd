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
    t_symbol * string;
    t_float start;
    t_float length;
    t_inlet * start_inlet;
    t_inlet * length_inlet;
    t_outlet * outlet;
} t_substr;

void substr_setup();

t_class * substr_class;

void * substr_new(t_symbol *s, int argc, t_atom * argv)
{
    t_substr * res = (t_sp_substr*) pd_new(substr_class);
    res->start = 0;
    res->length = -1;
    
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
    std::string s = f->s_name;
    // 
    if(x->start >= 0) { 
        if(x->start >= s.length()) {
            error("sp_substr: too big start position - %f", x->start);
            return;
        }
        
        size_t len = (x->length >= 0) ? x->length : std::string::npos;
        size_t start = x->start;
    
        s = s.substr(start, len);
        
        outlet_symbol(x->outlet, gensym(s.c_str()));    
    }
    else {
        long pos = s.length() + x->start;
        post("pos: %d", pos);
        
        size_t len = (x->length >= 0) ? x->length : std::string::npos;
        if(pos >= 0 && pos < s.length()) {
            s = s.substr(pos, len);
            outlet_symbol(x->outlet, gensym(s.c_str()));  
        }
    }
}

static void substr_bang(t_substr * x)
{
    // sp_update_freq(x);
    // outlet_float(x->outlet, x->out_freq);
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
    // class_addbang(sp_substr_class, sp_substr_bang);
}


