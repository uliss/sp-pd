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

#include "utf8.h"

long utf8_strlen(const char * str)
{
    const size_t ascii_len = strlen(str);
    
    long res = 0;
    for(size_t i = 0; i < ascii_len; i++) {
        unsigned int c = (unsigned char) str[i];
        if(c <= 127) i += 0;
        else if ((c & 0xE0) == 0xC0) i += 1;
        else if ((c & 0xF0) == 0xE0) i += 2;
        else if ((c & 0xF8) == 0xF0) i += 3;
        else return -1; // invalid utf-8 char
        
        res++;
    }
    
    return res;
}

int utf8_char_at(const char * str, size_t pos, char * dest)
{
    const size_t ascii_len = strlen(str);
    
    long res = 0;
    for(size_t i = 0; i < ascii_len; i++) {
        unsigned int c = (unsigned char) str[i];
        size_t offset;
        
        if(c <= 127) offset = 0;
        else if ((c & 0xE0) == 0xC0) offset = 1;
        else if ((c & 0xF0) == 0xE0) offset = 2;
        else if ((c & 0xF8) == 0xF0) offset = 3;
        else return -1; // invalid utf-8 char
        
        if(res == pos) { // position found
            memcpy(dest, str + i, offset + 1);
            dest[offset + 1] = '\0';
            return 0;
        }
        
        i += offset;
        res++;
    }
    
    return -1;
}

int utf8_char_width(const char * str)
{
    unsigned int c = (unsigned char) str[0];
    if(c <= 127)
        return 1;
    else if ((c & 0xE0) == 0xC0)
        return 2;
    else if ((c & 0xF0) == 0xE0) 
        return 3;
    else if ((c & 0xF8) == 0xF0) 
        return 4;
    else 
        return 0;
}

int utf8_substr(const char * str, size_t start, size_t length, char * buffer)
{
    if(length == 0) 
        return -1;
    
    const size_t ascii_len = strlen(str);
    
    long chars_copied = 0;
    long current_char = 0;
    for(size_t i = 0; i < ascii_len; current_char++) {
        size_t char_wd = utf8_char_width(str + i);
        if(!char_wd)
            break;
        
        if(current_char >= start) {
            if(chars_copied == length) {
                *buffer = '\0';
                break;
            }
            
            memcpy(buffer, str + i, char_wd);
            buffer += char_wd;
            chars_copied++;
        }
        
        i += char_wd;
    }
    
    *buffer = '\0';

    return 0;
}
