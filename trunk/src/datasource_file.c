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
#include <string.h>
#include "graph.h"
#include "fmem.h"
#include "datasource.h"
#include "datasource_file.h"
#include "visual.h"
#include "graphfile.h"

void *datasource_file_create( char *args );
void datasource_file_free( void *storage );
graph_t *datasource_file_get( void *storage );
void datasource_file_show( void *storage, graph_t *graph, graph_index_t *cliqueid );

const datasource_t datasource_file = {
    datasource_file_create,
    datasource_file_free,
    datasource_file_get,
    datasource_file_show
};

typedef struct datasource_file_storage_t {
    graph_t *graph;
} datasource_file_storage_t;


void *datasource_file_create( char *args ) {
    datasource_file_storage_t *s = fmem_alloc( sizeof( datasource_file_storage_t ) );
    if( strcmp( args, "-" ) == 0 ) {
        s->graph = graphfile_readfobj( stdin );
    } else {
        s->graph = graphfile_readfile( args );
    }
    return (void*)s;
}

void datasource_file_free( void *storage ) {
    fmem_free( storage );
}

graph_t *datasource_file_get( void *storage ) {
    graph_t *graph;
    datasource_file_storage_t *s = (datasource_file_storage_t *)storage;
    graph = s->graph;
    s->graph = NULL;
    return  graph;
}

void datasource_file_show( void *storage, graph_t *graph, graph_index_t *cliqueid ) {
    visual_show( graph, cliqueid );
}
