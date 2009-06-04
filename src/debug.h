/*
 * This file is part of cluster-editing-algorithms
 *
 * cluster-editing-algorithms is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * cluster-editing-algorithms is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cluster-editing-algorithms.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG

extern int g_debug_level;

#include <stdio.h>
#include <stdlib.h>

#define DBGPRINT( _LVL, _MSG ) {                                            \
    if( (_LVL) <= g_debug_level )                                           \
    fprintf( stderr, "DBG(%2d) %-20s @ %5d : %s\n", (_LVL), __FILE__, __LINE__, (_MSG) );\
}

#define DBGVAR( _LVL, _TYPE, _VAR ) {                                       \
    if( (_LVL)<=g_debug_level)                                              \
    fprintf( stderr, "DBG(%2d) %-20s @ %5d : %s = " _TYPE "\n",                  \
            (_LVL), __FILE__, __LINE__, #_VAR, (_VAR) );                            \
}

#define ASSERT( _EXPR ) {                                                   \
    if( !( _EXPR ) ) {                                                      \
        fprintf( stderr, "ASSERTION ERROR %-20s @ %5d : %s\n",              \
                __FILE__, __LINE__, #_EXPR );                               \
        exit(1);                                                            \
    }                                                                       \
}

#else


#define DBGPRINT( _LVL, _MSG )
#define DBGVAR( _LVL, _TYPE, _VAR )
#define ASSERT( _EXPR )

#endif


#define DBGINT( _LVL, _VAR )   DBGVAR( _LVL, "%d", _VAR )
#define DBGLONG( _LVL, _VAR )  DBGVAR( _LVL, "%ld", _VAR )
#define DBGSTR( _LVL, _VAR )   DBGVAR( _LVL, "%s", _VAR )
#define DBGFLOAT( _LVL, _VAR ) DBGVAR( _LVL, "%f", _VAR )
#define DBGDOUBLE( _LVL, _VAR ) DBGVAR( _LVL, "%f", _VAR )

#endif
