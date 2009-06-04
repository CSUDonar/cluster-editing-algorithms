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
#include "visual.h"
#include "fmem.h"

void visual_show( graph_t *graph, graph_index_t *cliqueid ) {
    graph_index_t i,j;
    graph_index_t c_id;
    graph_value_t val;
    c_id = 0;
    /* Fetch maximum clique id */
    for( i=graph_getNodeCount( graph )-1; i>=0; i-- ) {
        if( cliqueid[i]+1 > c_id ) {
            c_id = cliqueid[i]+1;
        }
    }
    /* Print cliques */
    for( i=0; i<c_id; i++ ) {
        printf( "%4ld =", i );
        for( j=0; j<graph_getNodeCount( graph ); j++ ) {
            if( cliqueid[j] == i ) {
                printf( " %3ld", j );
            }
        }
        printf( "\n" );
    }

    /* Print missmatches */
    for( i=0; i>=0; i = graph_getNext( graph, i ) ) {
        for( j=graph_getNext( graph, i ); j>=0; j = graph_getNext( graph, j ) ) {
            if( i != j ) {
                val = graph_getValue( graph, i, j );
                if( val>0 && cliqueid[i]!=cliqueid[j] ) {
                    printf( "Edge missmatch %ld and %ld, value: %ld\n", i, j, val );
                }
                if( val<0 && cliqueid[i]==cliqueid[j] ) {
                    printf( "Nonedge missmatch %ld and %ld, value: %ld\n", i, j, val );
                }
                if( val==0 ) {
                    if( cliqueid[i]==cliqueid[j] ) {
                        printf( "Zeroedge treated as edge %ld and %ld\n", i, j );
                    } else {
                        printf( "Zeroedge treated as non-edge %ld and %ld\n", i, j );
                    }
                }
            }
        }
    }
}
