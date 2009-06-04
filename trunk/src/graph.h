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
#ifndef GRAPH_H
#define GRAPH_H

#include "debug.h"

/* TODO: Make nicer */
#define GRAPH_VALUE_FORBIDDEN  (-100000L)
#define GRAPH_VALUE_PERSISTANT ( 100000L)

#define GRAPH_EDGE_IDX(n1,n2) ( (n1)<(n2) ? (((n2)*((n2)-1) >> 1)+(n1)) : (((n1)*((n1)-1) >> 1) + (n2)) )

typedef long graph_index_t;
typedef long graph_value_t;

typedef graph_value_t graph_cost_t;

typedef graph_index_t graph_size_t;


/* TODO: edges var needed? */ 

typedef struct graph_t {
    graph_size_t    nodes;
    graph_value_t   *edges;
    graph_index_t   *node;
#if DEBUG
    int state_last; /* Enumerate changesets to test for implementation errors */
    int state_current;
#endif
} graph_t;

typedef struct graph_chSet_t graph_chSet_t;

struct graph_chSet_t {
                                    /* Function to apply changeset */
    graph_cost_t (*func)( graph_t *, graph_chSet_t *, graph_cost_t, graph_cost_t );

    graph_index_t       n1, n2;     /* Nodes involved in changeset */
    union {
        graph_value_t       value;      /* When setedge, New value of edge, when setedge */
        graph_index_t       prev;       /* When merged, contains previous node to n2 */
    } type_spec;
#if DEBUG
    int state_id;
    int state_target;
#endif
};

graph_t *graph_create( graph_size_t nodes );

void graph_free( graph_t *graph );

graph_value_t graph_getValue( const graph_t *graph, graph_index_t n1, graph_index_t n2 );

graph_size_t graph_getNodeCount( const graph_t *graph );
graph_size_t graph_getEdgeCount( const graph_t *graph );

graph_index_t graph_getNext( const graph_t *graph, graph_index_t i );

void graph_setValue( graph_t *graph, graph_index_t n1, graph_index_t n2, graph_value_t val );

void graph_setAllCosts( graph_t *graph, graph_value_t val );

int graph_isClusterGraph( const graph_t *graph);


/* changeset handling.
 * Doesn't modify graph itself. (exception: debug counter)
 */
graph_chSet_t *graph_merge(         const graph_t *graph, graph_index_t n1, graph_index_t n2 );
graph_chSet_t *graph_setForbidden(  const graph_t *graph, graph_index_t n1, graph_index_t n2 );
graph_chSet_t *graph_setPersistant( const graph_t *graph, graph_index_t n1, graph_index_t n2 );
void graph_chSet_free( graph_chSet_t *chSet );

/* Apply changeset
 * changeset is changed to it's inverse.
 * When DEBUG is set, check that changesets apply to correct state
 */
graph_cost_t graph_apply( graph_t *graph, graph_chSet_t *changeset, graph_cost_t fixpoint, graph_cost_t bookkeepingValue );


/* Internal chSet handlers: DO NOT CALL DIRECTLY */

graph_cost_t graph_apply_merge(   graph_t *graph, graph_chSet_t *chs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue );
graph_cost_t graph_apply_split(   graph_t *graph, graph_chSet_t *chs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue );
graph_cost_t graph_apply_setEdge( graph_t *graph, graph_chSet_t *chs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue );


#endif

