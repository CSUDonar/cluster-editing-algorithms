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
#include "graphstate.h"
#include "fmem.h"

graphstate_t *graphstate_create_base( graph_t *graph, graph_cost_t cost_left ) {
    graphstate_t *graphstate = fmem_alloc( sizeof( graphstate_t ) );

    graphstate->type = GRAPHSTATE_TYPE_ORG;
    /* Always start with one reference,
       the internal of the calling function */
    graphstate->references = 1;
    graphstate->c.org = fmem_alloc( sizeof( graphstate_org_t ) );

    graphstate->c.org->graph = graph;;

    graphstate->cost = 0; /* Start cost */
    graphstate->cost_left = cost_left;

    DBGPRINT( 20, "creating base" );


    return graphstate;
}

graphstate_t *graphstate_create_chset( graphstate_t *target, graph_chSet_t *chset ) {
    graphstate_t *graphstate = fmem_alloc( sizeof( graphstate_t ) );

    graphstate->type = GRAPHSTATE_TYPE_CHANGESET;
    /* Always start with one reference,
       the internal of the calling function */
    graphstate->references = 1;
    graphstate->c.chset = fmem_alloc( sizeof( graphstate_chset_t ) );

    graphstate->c.chset->target = target;
    graphstate_incref( graphstate->c.chset->target );

    graphstate->c.chset->chset = chset;

    graphstate->cost = -1; /* negative = invalid/unset */
    graphstate->cost_left = -1;

    DBGPRINT( 21, "creating chset" );

    return graphstate;
}

void graphstate_garbage_collect( graphstate_t *graphstate ) {
    graphstate_t *next;

    while( graphstate != NULL && graphstate->references <= 0 ) {
        ASSERT( graphstate->references == 0 /* less than 0 == error */ );
        if( graphstate->type == GRAPHSTATE_TYPE_CHANGESET ) {
            next = graphstate->c.chset->target;

            graph_chSet_free( graphstate->c.chset->chset );
            fmem_free( graphstate->c.chset );

            DBGPRINT( 21, "removing chset" );

            next->references--; /* Do not use graphstate_decref to avoid recursion */
        } else if( graphstate->type == GRAPHSTATE_TYPE_ORG ) {
            /* Never free datastructure */
            fmem_free( graphstate->c.org );

            DBGPRINT( 20, "removing base" );

            next = NULL;
        } else {
            DBGINT( 0, graphstate->type );
            ASSERT( 0 );
        }
        fmem_free( graphstate );

        graphstate = next;
    }
}

void graphstate_fetch( graphstate_t *graphstate, graph_cost_t fixpoint, graph_cost_t bookkeepingValue ) {
    graphstate_t        *target;
    graphstate_chset_t  *chset;
    graphstate_org_t    *org;
    graph_cost_t        cost;
    if( graphstate->type == GRAPHSTATE_TYPE_CHANGESET ) {
        target = graphstate->c.chset->target;

        graphstate_fetch( target, fixpoint, bookkeepingValue ); /* TODO: Make non-recursive */

        /* target will point to graphstate */
        graphstate_incref( graphstate );

        org = target->c.org;
        chset = graphstate->c.chset;

        cost = graph_apply( org->graph, chset->chset, fixpoint, bookkeepingValue );

        target->type = GRAPHSTATE_TYPE_CHANGESET;
        target->c.chset = chset;
        target->c.chset->target = graphstate;

        graphstate->type = GRAPHSTATE_TYPE_ORG;
        graphstate->c.org = org;

        /* Update next cost, if not updated before.
         * (This test makes it unnessecary to calculate reverse cost when
         * reversing edits, like splitting merged nodes
         */
        if( graphstate->cost < 0 ) {
            graphstate->cost      = target->cost + cost;
            graphstate->cost_left = target->cost_left - cost;
        }

        /* graphstate doesn't point to target anymore */
        graphstate_decref( target );
    }
}

void graphstate_lock( graphstate_t *graphstate, graph_cost_t fixpoint, graph_cost_t bookkeepingValue ) {
    /* TODO: make threadsafe */
    graphstate_fetch( graphstate, fixpoint, bookkeepingValue );
}

void graphstate_unlock( graphstate_t *graphstate ) {
    /* TODO: make threadsafe */
}

void graphstate_incref( graphstate_t *graphstate ) {
    graphstate->references++;
}
void graphstate_decref( graphstate_t *graphstate ) {
    graphstate->references--;
    graphstate_garbage_collect( graphstate ); /* Possibly remove object */
}


void graphstate_tracemerges( graphstate_t *basestate, graph_index_t *idlist ) {
    graph_chSet_t *chset;

    /* FIXME: make nonrecurive */
    if( basestate->type == GRAPHSTATE_TYPE_CHANGESET ) {
        DBGPRINT( 22, "Tracing" );
        graphstate_tracemerges( basestate->c.chset->target, idlist );

        chset = basestate->c.chset->chset;

        /* If merge, propagate id to child node */
        if( chset->func == graph_apply_split ) { /* FIXME: Do not compare pointers!!! */
            idlist[chset->n2] = idlist[chset->n1];
            DBGLONG( 22, chset->n1 );
            DBGLONG( 22, chset->n2 );
        }
    }
}
