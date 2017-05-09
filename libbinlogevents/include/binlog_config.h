/**
 Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef BAPI_CONFIG_INCLUDED
#define BAPI_CONFIG_INCLUDED
/* Headers we may use */
#define HAVE_STDINT_H 1
/* #undef HAVE_ENDIAN_H */
/* Symbols we may use */
#define STANDALONE_BINLOG TRUE
/* #undef IS_BIG_ENDIAN */
/* #undef HAVE_LE64TOH */
/* #undef HAVE_LE32TOH */
/* #undef HAVE_LE16TOH */
#define HAVE_STRNDUP 1
/* #undef HAVE_ENDIAN_CONVERSION_MACROS */
/* #undef SIZEOF_LONG_LONG */
#define HAVE_LONG_LONG 1
/* #undef SIZEOF_LONG */
#define HAVE_LONG 1
/* #undef SIZEOF_INT */
#define HAVE_INT 1

#endif
