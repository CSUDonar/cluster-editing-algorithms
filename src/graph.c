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

#include "debug.h"
#include "graph.h"
#include "fmem.h"

/* TODO: Indexing node vector shares code with set all costs */
    
graph_t *graph_create( graph_size_t nodes ) {
    graph_size_t    edges;
    graph_index_t   i;
    graph_t         *graph;

    graph = fmem_alloc(sizeof(graph_t));
    graph->nodes = nodes;

    edges = GRAPH_EDGE_IDX( nodes, 0 ); /* (nodes * (nodes - 1) >> 1); */

    if( graph->nodes == 0 ) {
        graph->edges = NULL;
        graph->node = NULL;
    } else {
        graph->edges = fmem_alloc_arr(sizeof(graph_value_t), edges);
        for (i = 0; i < edges; i++) {
            graph->edges[i] = -1;
        }

        /*
         * A dummy node is inserted in the nodes array
         * to ensure that we always can find the first existing node
         */
        graph->node = fmem_alloc_arr(sizeof(graph_index_t), nodes);

        for(i=0; i<nodes-1; i++) 
            graph->node[i] = i+1;
        
        graph->node[nodes-1] = -1;
    }
    return graph;
}

void graph_free( graph_t *graph ) {
    fmem_free(graph->edges);
    fmem_free(graph->node);
    fmem_free(graph);
}

graph_value_t graph_getValue(   const graph_t *graph,
                                graph_index_t n1,
                                graph_index_t n2) {
    ASSERT( n1 != n2 );
    return graph->edges[GRAPH_EDGE_IDX( n1, n2 ) ];
}

graph_size_t graph_getNodeCount( const graph_t *graph ) {
    return graph->nodes;
}

graph_size_t graph_getEdgeCount( const graph_t *graph ) {
    return GRAPH_EDGE_IDX( graph->nodes, 0 ); /* TODO: Nicer way? */
}

graph_index_t graph_getNext( const graph_t *graph, graph_index_t i ) {
    return graph->node[i];
}

void graph_setValue( graph_t *graph,
                     graph_index_t n1,
                     graph_index_t n2,
                     graph_value_t val ) {
    ASSERT(n1 != n2);
    graph->edges[GRAPH_EDGE_IDX(n1,n2)] = val;
}

void graph_setAllCosts( graph_t *graph, graph_value_t val ) {
    graph_index_t i, j;
    for( i=0; i<graph->nodes; i++ ) {
        for( j=i+1; j<graph->nodes; j++ ) {
            graph->edges[ GRAPH_EDGE_IDX( i, j ) ] = val;
        }
        graph->node[i] = i+1;
    }
    graph->node[graph->nodes-1] = -1;
}

void graph_chSet_free( graph_chSet_t *chSet ) {
    fmem_free( chSet );
}

/* 
 * All information about n2 is saved in n1 and then we 
 * change the linking of the node array to skip n2 
 */
graph_chSet_t *graph_merge( const graph_t *graph, graph_index_t n1, graph_index_t n2 ) {
    graph_chSet_t *chs;

    chs = fmem_alloc( sizeof( graph_chSet_t ) );

    ASSERT( n1 != n2 );

    chs->func = graph_apply_merge;
    if( n1 < n2 ) {
        chs->n1 = n1;
        chs->n2 = n2;
    } else {
        chs->n1 = n2;
        chs->n2 = n1;
    }

    return chs;
}

/*
 * dummy value -100000 is used instead of INT_MIN because of 
 * overflow/wraparound problems when using it in calculations
 */

graph_chSet_t *graph_setForbidden( const graph_t *graph, graph_index_t n1, graph_index_t n2 ) {
    graph_chSet_t *chs;
    chs = fmem_alloc(sizeof(graph_chSet_t));

    ASSERT( n1 != n2 );

    chs->func = graph_apply_setEdge;
    if( n1 < n2 ) {
        chs->n1 = n1;
        chs->n2 = n2;
    } else {
        chs->n1 = n2;
        chs->n2 = n1;
    }
    chs->type_spec.value = GRAPH_VALUE_FORBIDDEN;

    return chs;
}

/*TODO Stavas Persistent*/
graph_chSet_t *graph_setPersistant( const graph_t *graph, graph_index_t n1, graph_index_t n2 ) {
    graph_chSet_t *chs;
    chs = fmem_alloc(sizeof(graph_chSet_t));

    ASSERT( n1 != n2 );

    chs->func = graph_apply_setEdge;
    if( n1 < n2 ) {
        chs->n1 = n1;
        chs->n2 = n2;
    } else {
        chs->n1 = n2;
        chs->n2 = n1;
    }
    chs->type_spec.value = GRAPH_VALUE_PERSISTANT;

    return chs;
}



graph_cost_t graph_apply_merge(   graph_t *graph, graph_chSet_t *chs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue ) {
    graph_index_t       edge_idx;
    graph_index_t       i;
    graph_index_t       ei1, ei2; /* Edge index */

    graph_value_t       ev1, ev2;

    graph_cost_t        cost;
    edge_idx = GRAPH_EDGE_IDX( chs->n1, chs->n2 );

    cost = 0;

    /* If nonedge merging, add cost for it */
    ev1 = graph->edges[GRAPH_EDGE_IDX(chs->n1, chs->n2)];
    if( ev1 < 0 ) {
        cost += -ev1*fixpoint;
    } else if( ev1 == 0 ) {
        cost += bookkeepingValue; /* Resolved a zeroedge */
    }
    
    /* Merge values, reversable due to storage of n2 */
    for( i=0; i>=0; i=graph->node[i] ) {
        if( i != chs->n1 && i != chs->n2 ) {
            ei1 = GRAPH_EDGE_IDX(chs->n1,i);
            ei2 = GRAPH_EDGE_IDX(chs->n2,i);

            ev1 = graph->edges[ei1];
            ev2 = graph->edges[ei2];

            if( (ev1 < 0) && (ev2 > 0) ) {
                if( -ev1 < ev2 ) {
                    cost += -ev1*fixpoint;
                } else {
                    cost += ev2*fixpoint;
                }
            } else if( (ev1 > 0) && (ev2 < 0) ) {
                if( ev1 < -ev2 ) {
                    cost += ev1*fixpoint;
                } else {
                    cost += -ev2*fixpoint;
                }
            }

            if( ev1 + ev2 == 0 ) {
                /* Bookkeeping */
                if( ev1 == 0 && ev2 == 0 ) {
                    /* two zero-edges resolved, one created */
                    cost += bookkeepingValue;
                } 
                else {
                    cost -= bookkeepingValue;
                }
            }
            else if (ev1 == 0 || ev2 == 0) {
                cost += bookkeepingValue;
            } 

            graph->edges[ei1] += ev2;
        }
    }

    /* Skip n2 */
    i=0;
    while( graph->node[i] != chs->n2 ) {
        i = graph->node[i];
    }
    chs->type_spec.prev = i;
    graph->node[i] = graph->node[chs->n2];

    /* Update changeset */
    chs->func = graph_apply_split;

    return cost;
}

graph_cost_t graph_apply_split(   graph_t *graph, graph_chSet_t *chs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue ) {
    graph_index_t       edge_idx;
    graph_index_t       i;
    graph_index_t       ei1, ei2; /* Edge index */

    edge_idx = GRAPH_EDGE_IDX( chs->n1, chs->n2 );

    /* Merge values, reversable due to storage of n2 */
    for( i=0; i>=0; i=graph->node[i] ) {
        if( i != chs->n1 && i != chs->n2 ) {
            ei1 = GRAPH_EDGE_IDX(chs->n1,i);
            ei2 = GRAPH_EDGE_IDX(chs->n2,i);

            graph->edges[ei1] -= graph->edges[ei2];

            /* Can only happen when reversing edits; no need to recalculate */
        }
    }

    /* Insert n2 in list */
    graph->node[chs->type_spec.prev] = chs->n2;

    /* Update changeset */
    chs->func=graph_apply_merge;

    return 0; /* Ignore costs here, this is only used for reversing changes... */
}

graph_cost_t graph_apply_setEdge( graph_t *graph, graph_chSet_t *chs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue ) {
    graph_index_t       edge_idx;
    graph_index_t       old_v, new_v; /* Edge values */

    graph_cost_t        cost;

    cost = 0;

    edge_idx = GRAPH_EDGE_IDX( chs->n1, chs->n2 );

    old_v = graph->edges[edge_idx];
    new_v = chs->type_spec.value;
    graph->edges[edge_idx] = new_v;
    chs->type_spec.value = old_v;

    DBGLONG( 12, old_v );
    DBGLONG( 12, new_v );

    if( old_v<0 && new_v>0 ) {
        cost += -old_v*fixpoint;
    } else if( old_v>0 && new_v<0 ) {
        cost += old_v*fixpoint;
    }
    if( old_v == 0 ) {
        /* Bookkeeping: Resolving a zero edge */
        cost += bookkeepingValue;
    }
    if( new_v == 0 ) {
        /* Bookkeeping: Creating a zero edge */
        cost -= bookkeepingValue;
    }

    return cost;
}

graph_cost_t graph_apply( graph_t *graph, graph_chSet_t *chs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue ) {
    return (*chs->func)( graph, chs, fixpoint, bookkeepingValue );
}

int graph_isClusterGraph( const graph_t *g){
    int n1, n2, n3;
    
    for (n1 = 0; n1 >= 0; n1 = graph_getNext(g, n1) ){
        for (n2=graph_getNext(g, n1); n2>= 0; n2 = graph_getNext(g, n2) ){
            if( graph_getValue( g, n1, n2 ) < 0 ) {
                for (n3=graph_getNext(g, n2); n3 >= 0; n3 = graph_getNext(g, n3) ){
                    if(     graph_getValue( g, n1, n3 ) > 0 &&
                            graph_getValue( g, n2, n3 ) > 0 ) {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

