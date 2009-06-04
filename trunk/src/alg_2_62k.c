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
#include <stdio.h>

#include "debug.h"
#include "sched.h"
#include "alg_2_62k.h"

#include "graphstate.h"

#include "kernel.h"

void alg_2_62k_calculate( sched_t *sched, void *job );
void alg_2_62k_job_free( sched_t *sched, void *job );
int alg_2_62k_job_compare( sched_t *sched, void *joba, void *jobb );
void alg_2_62k_inc_limit_job( sched_t *sched, void *job );

sched_algorithm_t alg_2_62k = {
    alg_2_62k_calculate,
    alg_2_62k_job_free,
    alg_2_62k_job_compare,
    alg_2_62k_inc_limit_job
};

void alg_2_62k_calculate( sched_t *sched, void *job ) {
    graphstate_t *graphstate = (graphstate_t*)job;
    graphstate_t *tmp;
    graph_t *g;
    graph_index_t a,b,c;
    graph_cost_t cost_left;

    graphstate_lock( graphstate, 1, 0 );
    cost_left = graphstate->cost_left;
    graphstate_unlock( graphstate );
    if( cost_left < 0 ) {
        graphstate_decref( graphstate );
        return;
    }

    graphstate = kernel_kernelize( graphstate, 1, 0 );

    tmp = (graphstate_t *)sched_getBest( sched );
    graphstate_lock( graphstate, 1, 0 );
    if( graphstate->cost_left < 0 ) { /* If no cost is left, leave it */
        graphstate_unlock( graphstate );
        graphstate_decref( graphstate );
        return;
    }

    if( graphstate->cost_left >= 0 && (tmp == NULL || graphstate->cost < tmp->cost ) ) {;

        g = graphstate->c.org->graph;
        for( a = 0; a >= 0; a = graph_getNext( g, a ) ) {
            for( b = graph_getNext( g, a ); b >= 0; b = graph_getNext( g, b ) ) {
                if( graph_getValue( g,a,b ) <= 0 ) {
                    for( c = 0; c >= 0; c = graph_getNext( g, c ) ) {
                        if(     ( a!=c && b!=c ) && (
                                (
                                    ( graph_getValue( g, a, c ) >  0 ) &&
                                    ( graph_getValue( g, b, c ) >= 0 )
                                ) || (
                                    ( graph_getValue( g, a, c ) >= 0 ) &&
                                    ( graph_getValue( g, b, c ) >  0 )
                                ) 
                                ) ) {
                            DBGLONG( 11, a );
                            DBGLONG( 11, b );
                            DBGLONG( 11, c );

                            tmp = graphstate_create_chset( graphstate,
                                    graph_merge( g, a, c ) );
                            sched_job_add( sched, tmp );

                            tmp = graphstate_create_chset( graphstate,
                                    graph_setForbidden( g, a, c ) );
                            sched_job_add( sched, tmp );

                            DBGLONG( 11, graphstate->cost );

                            goto alg_2_62k_dengo;
                            /* TODO: Kill the velociraptor */
                        }
                    }
                }
            }
        }


        DBGLONG( 10, graphstate->cost );

        /* Set best, and if updated, fix refcounters */
        if( sched_setBest( sched, graphstate, (void**)&tmp ) ) {
            if( tmp != NULL ) {
                graphstate_decref( tmp );
            }
            graphstate_incref( graphstate );
        }

    alg_2_62k_dengo:

        ;

    } else {

        DBGPRINT( 11, "Worse than best" );
    }

    graphstate_unlock( graphstate );
    graphstate_decref( graphstate );
}

void alg_2_62k_job_free( sched_t *sched, void *job ) {
    graphstate_decref( (graphstate_t*)job );
}

int alg_2_62k_job_compare( sched_t *sched, void *joba, void *jobb ) {
    graphstate_t *gsa = (graphstate_t*)joba;
    graphstate_t *gsb = (graphstate_t*)jobb;
    ASSERT( gsa->cost >= 0 );
    ASSERT( gsb->cost >= 0 );
    return gsa->cost - gsb->cost;
}

void alg_2_62k_inc_limit_job( sched_t *sched, void *job ) {
    graphstate_t *gs = (graphstate_t*)job;
    gs->cost_left += 1;
}
