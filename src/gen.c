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
#include <time.h>
#include "graph.h"
#include "gen.h"
#include "fmem.h"

/*
* "nodes"       - desired number of nodes
* "cliques"     - desired number of cliques
* "edits"       - number of edits to be made on the graph
* "maxWeight"   - edges will have costs between -maxWeight and maxWeight
*
* Creates a graph with given attributes and makes "edits" arbitrary edits on it
*/

graph_t *gen_generate(int nodes, int cliques, int edits, int maxWeight) {
    graph_index_t *cliqid;
    graph_index_t i,j,k;
    graph_value_t val;

    graph_t *graph;

    cliqid = fmem_alloc_arr( sizeof( graph_index_t ), nodes );

    /* Give nodes a clique number with even distribution */
    for( i=0; i<nodes; i++ ) {
        cliqid[i] = i%cliques;
    }

    /* scramble */
    for( i=0; i<nodes; i++ ) {
        j = rand() % nodes;

        /* swap i and j */
        k = cliqid[i];
        cliqid[i] = cliqid[j];
        cliqid[j] = k;
    }

    /* Build a perfect cluster graph */
    graph = graph_create( nodes );
    for( i=0; i<nodes; i++ ) {
        for( j=i+1; j<nodes; j++ ) {
            val = (rand() % maxWeight) + 1;
            if( cliqid[i] != cliqid[j] ) {
                val = -val;
            }
            graph_setValue( graph, i, j, val );
        }
    }

    /* Add noise */
    while( edits > 0 ) {
        i=rand()%nodes;
        j=rand()%nodes;
        if( i!= j ) {
            val = graph_getValue( graph, i, j );
            /* Do not edit changed edges */
            if(     (val<0 && cliqid[i]!=cliqid[j]) ||
                    (val>0 && cliqid[i]==cliqid[j]) ) {
                if( val > 0 ) {
                    graph_setValue( graph, i, j, -maxWeight/2 );
                } else {
                    graph_setValue( graph, i, j, maxWeight/2 );
                }
                edits--;
            }
        }
    }

    fmem_free( cliqid );

    return graph;
}

