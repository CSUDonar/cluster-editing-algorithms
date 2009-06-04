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
#include <stdio.h>
#include "graph.h"
#include "graphstate.h"
#include "postprocess.h"
#include "fmem.h"

graph_index_t *postprocess_enumerate_cliques( graph_t *graph, graphstate_t *basestate ) {
    graph_index_t *cliqueid;

    graph_index_t c_id,i,j;

    if( graph_getNodeCount( graph ) > 0 ) {
        cliqueid = fmem_alloc_arr( sizeof( graph_index_t ), graph_getNodeCount( graph ) );

        for( i=graph_getNodeCount( graph )-1; i>=0; i-- ) {
            cliqueid[i] = -1;
        }

        /* enumerate cliques */
        c_id = 0;
        for( i=0; i>=0; i = graph_getNext( graph, i ) ) {
            if( cliqueid[i] < 0 ) {
                cliqueid[i] = c_id;
                for( j=0; j>=0; j = graph_getNext( graph, j ) ) {
                    if( i != j ) {
                        if( cliqueid[j] < 0 && graph_getValue( graph, i, j ) >= 0 ) {
                            cliqueid[j] = c_id;
                        }
                    }
                }
                c_id++;
            }
        }

        /* Track merges */
        graphstate_tracemerges( basestate, cliqueid );
    } else {
        cliqueid = fmem_alloc_arr( sizeof( graph_index_t ), 1 );
    }

    return cliqueid;
}
