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
#include "debug.h"
#include "fmem.h"
#include "gen.h"
#include "datasource.h"
#include "datasource_random.h"
#include "visual.h"

void *datasource_random_create( char *args );
void datasource_random_free( void *storage );
graph_t *datasource_random_get( void *storage );
void datasource_random_show( void *storage, graph_t *graph, graph_index_t *cliqueid );

const datasource_t datasource_random = {
    datasource_random_create,
    datasource_random_free,
    datasource_random_get,
    datasource_random_show
};

typedef struct datasource_random_storage_t {
    int generated;
    graph_index_t   numNodes;
    graph_index_t   numCliques;
    graph_index_t   numNoise;
    graph_value_t   maxWeight;
} datasource_random_storage_t;


void *datasource_random_create( char *args ) {
    char *tok;
    datasource_random_storage_t *s = fmem_alloc( sizeof( datasource_random_storage_t ) );
    s->generated = 0;

    s->numNodes     = 100;
    s->numCliques   =  10;
    s->numNoise     =   7;
    s->maxWeight    =   3;


    if( args != NULL ) {
        if( (tok = strtok( args, ":" )) != NULL ) {
            s->numNodes = atol( tok );
            if( (tok = strtok( NULL, ":" )) != NULL ) {
                s->numCliques = atol( tok );
                if( (tok = strtok( NULL, ":" )) != NULL ) {
                    s->numNoise = atol( tok );
                    if( (tok = strtok( NULL, ":" )) != NULL ) {
                        s->maxWeight = atol( tok );
                    }
                }
            }
        }
    }

    DBGLONG( 2, s->numNodes );
    DBGLONG( 2, s->numCliques );
    DBGLONG( 2, s->numNoise );
    DBGLONG( 2, s->maxWeight );

    return (void*)s;
}

void datasource_random_free( void *storage ) {
    fmem_free( storage );
}

graph_t *datasource_random_get( void *storage ) {
    datasource_random_storage_t *s = (datasource_random_storage_t *)storage;
    if( s->generated ) {
        return NULL;
    }
    s->generated = 1;
    return  gen_generate(
            s->numNodes,
            s->numCliques,
            s->numNoise,
            s->maxWeight
            );
}

void datasource_random_show( void *storage, graph_t *graph, graph_index_t *cliqueid ) {
    visual_show( graph, cliqueid );
}
