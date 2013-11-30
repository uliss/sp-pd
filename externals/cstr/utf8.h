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

#include <stddef.h>

/**
 * Returns numbers of chars in utf-8 encoded string.
 * @param str - utf-8 string
 * @return number of chars on success, or -1 on error.
 */
long utf8_strlen(const char * str);

/**
 * Writes to destination utf-8 encoded char at given position.
 * @param str - source utf-8 encoded string
 * @param pos - character position
 * @param dest - pointer to write result. Must have enough space to store
 *               utf-8 character + trailing '\0' == 5 bytes.
 * @return 0 on sucess, -1 on error
 */
int utf8_char_at(const char * str, size_t pos, char * dest);

/**
 * Returns substring from utf-8 encoded string.
 * @param str - source string
 * @param pos - substring start
 * @param length - substring length
 * @param buffer - pointer to write buffer. Must have enough space to store
 *               utf-8 substring + trailing '\0'.
 * @return 0 on success, -1 on error
*/
int utf8_substr(const char * str, size_t pos, size_t length, char * buffer);