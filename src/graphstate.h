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
#ifndef GRAPHSTATE_H
#define GRAPHSTATE_H

#include "graph.h"

#define GRAPHSTATE_TYPE_ORG       1
#define GRAPHSTATE_TYPE_CHANGESET 2

typedef struct graphstate_t {
    int references;
    int type;
    graph_cost_t cost;
    graph_cost_t cost_left;
    union {
        struct graphstate_org_t     *org;
        struct graphstate_chset_t   *chset;
    } c;
} graphstate_t;

typedef struct graphstate_org_t {
    graph_t *graph;
} graphstate_org_t;

typedef struct graphstate_chset_t {
    graphstate_t *target;
    graph_chSet_t *chset;
} graphstate_chset_t;


/* Creates a graphstate_t changeset node with a local reference
 */
graphstate_t *graphstate_create_base( graph_t *graph, graph_cost_t cost_left );
graphstate_t *graphstate_create_chset(
	graphstate_t *target,
	graph_chSet_t *chset );

/* Garbate collect: Almost as free, but free only if no references to object
 */
void graphstate_garbage_collect( graphstate_t *graphstate );

void graphstate_reverse( graphstate_t *graphstate );

/* Fetch and lock and instance of graphstate.
 * Between lock and unlock graphstate is always type org.
 * TODO: Make thread safe
 */
void graphstate_lock( graphstate_t *graphstate, graph_cost_t fixpoint, graph_cost_t bookkeepingValue );
void graphstate_unlock( graphstate_t *graphstate );

void graphstate_fetch( graphstate_t *graphstate, graph_cost_t fixpoint, graph_cost_t bookkeepingValue );

/* Increment and decrement reference counters.
 * When creating a reference, use incref.
 * When removing a reference, use decref.
 *
 * Make sure to use even for local references.
 */
void graphstate_incref( graphstate_t *graphstate );
void graphstate_decref( graphstate_t *graphstate );

void graphstate_tracemerges( graphstate_t *basestate, graph_index_t *idlist );

#endif
