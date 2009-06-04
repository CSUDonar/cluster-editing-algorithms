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
#include <stdlib.h>
#include "graph.h"
#include "fmem.h"
#include "graphstate.h"
#include "datasource.h"

datasource_storage_t *datasource_create( const datasource_t *source, char *args ) {
    datasource_storage_t *s = fmem_alloc( sizeof( datasource_storage_t ) );
    s->datasource = source;
    s->storage = (*s->datasource->create)( args );
    return s;
}

void datasource_free( datasource_storage_t *s ) {
    (s->datasource->free)( s->storage );
    fmem_free( s );
}

graph_t *datasource_get( datasource_storage_t *s ) {
    return (*s->datasource->get)( s->storage );
}
void datasource_show( datasource_storage_t *s, graph_t *graph, graph_index_t *cliqueid ) {
    (*s->datasource->show)( s->storage, graph, cliqueid );
}
