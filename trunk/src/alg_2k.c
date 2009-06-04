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
#include <math.h>

#include "debug.h"
#include "sched.h"
#include "alg_2k.h"
#include "fmem.h"

#include "graphstate.h"

#include "kernel.h"

void alg_2k_calculate( sched_t *sched, void *job );
void alg_2k_job_free( sched_t *sched, void *job );
int alg_2k_job_compare( sched_t *sched, void *joba, void *jobb );
void alg_2k_inc_limit_job( sched_t *sched, void *job );
double alg_2k_calcbranch( graph_cost_t ac, graph_cost_t bc );

sched_algorithm_t alg_2k = {
    alg_2k_calculate,
    alg_2k_job_free,
    alg_2k_job_compare,
    alg_2k_inc_limit_job
};

void alg_2k_calculate( sched_t *sched, void *job ) {
    graphstate_t *graphstate = (graphstate_t*)job;
    graphstate_t *tmp;
    graph_t *g;
    graph_index_t a,b,c;
    graph_cost_t ca, cb;
    graph_cost_t cmerge, cforbid;

    graph_cost_t *mergecost;
    double branchvec;
    double branchvecmin;
    graph_index_t mina, minb;

    graph_cost_t cost_left;

    graphstate_lock( graphstate, 2, 1 );
    cost_left = graphstate->cost_left;
    graphstate_unlock( graphstate );
    if( cost_left < 0 ) {
        graphstate_decref( graphstate );
        return;
    }

   graphstate = kernel_kernelize( graphstate, 2, 1 );

    tmp = (graphstate_t *)sched_getBest( sched );
    graphstate_lock( graphstate, 2, 1 );
    if( graphstate->cost_left < 0 ) { /* If no cost is left, leave it */
        graphstate_unlock( graphstate );
        graphstate_decref( graphstate );
        return;
    }

    if( graphstate->cost_left >= 0 && ( tmp == NULL || graphstate->cost < tmp->cost ) ) {

        DBGPRINT( 15, "New job" );

        g = graphstate->c.org->graph;

        /* TODO: no reallocation */
        mergecost = fmem_alloc_arr( sizeof( graph_cost_t ), graph_getEdgeCount( g ) );

        /* Initialize merge cost content */
        for( a = 0; a >= 0; a = graph_getNext( g, a ) ) {
            for( b = graph_getNext( g, a ); b >= 0; b = graph_getNext( g, b ) ) {
                if( a != b ) {
                    cmerge = graph_getValue( g, a, b );
                    if( cmerge == 0 ) {
                        mergecost[ GRAPH_EDGE_IDX( a, b ) ] = 1;
                    } else if( cmerge < 0 ) {
                        mergecost[ GRAPH_EDGE_IDX( a, b ) ] = -cmerge*2;
                    } else {
                        mergecost[ GRAPH_EDGE_IDX( a, b ) ] = 0;
                    }
                }
            }
        }

        /* Calculate merge costs in O(N^3) */

        /* Iterate through all edges (a,b) */
        for( a = 0; a >= 0; a = graph_getNext( g, a ) ) {
            for( b = graph_getNext( g, a ); b >= 0; b = graph_getNext( g, b ) ) {
                if( a != b ) {
                    /* Iterate through all other nodes, c */
                    for( c = 0; c >= 0; c = graph_getNext( g, c ) ) {
                        if( a != c && b != c ) {
                            /* Get costs of the edges (a,c) and (b,c) */
                            ca = graph_getValue( g, a, c );
                            cb = graph_getValue( g, b, c );


                            /* Calculate cost for merging (a,c) and (b,c), and add to (a,b) */
                            if( ca == 0 || cb == 0 ) {
                                /* One edge is zero-edge = that edge is resolved
                                 * Two edges is zero-edges = both edges is resolved, one new zerdo-edge is created
                                 * Both cases means one zero-edge is removed...
                                 */
                                mergecost[ GRAPH_EDGE_IDX( a, b ) ] += 1; /* Cost for resolving a zeroedge */
                                DBGPRINT( 19, "Zeroedge" );
                            }

                            if( ((ca<0) && (cb>0)) || ((ca>0) && (cb<0)) ) {
                                if( ca < 0 ) {
                                    ca = -ca;
                                }
                                if( cb < 0 ) {
                                    cb = -cb;
                                }
                                cmerge = (ca>cb) ? (cb) : (ca);

                                mergecost[ GRAPH_EDGE_IDX( a, b ) ] += cmerge*2;
                            }
                        }
                    }
                }
            }
        }

        /* Calculate branching vectors and select minimum*/
        mina = -1;
        for( a = 0; a >= 0; a = graph_getNext( g, a ) ) {
            for( b = graph_getNext( g, a ); b >= 0; b = graph_getNext( g, b ) ) {
                if( a != b ) {
                    cmerge = mergecost[ GRAPH_EDGE_IDX( a, b ) ];
                    /* TODO: cmerge == 0 means infinite branching vector; no branching */
                    if( cmerge > 0 ) {
                        cforbid = graph_getValue( g, a, b )*2;
                        if( cforbid >= 0 ) { /* Only handle zero-edges and edges */
                            if( cforbid == 0 ) {
                                /* Forbidding a zero-edge resolves it */
                                cforbid += 1;
                            }

                            branchvec = alg_2k_calcbranch( cmerge, cforbid );

                            DBGDOUBLE( 19, branchvec );

                            if( mina < 0 || branchvecmin > branchvec ) {
                                branchvecmin = branchvec;
                                mina = a;
                                minb = b;
                            }
                        }
                    }
                }
            }
        }

        if( mina >= 0 ) {
            sched_job_add( sched, (void*)graphstate_create_chset( graphstate, graph_merge( g, mina, minb ) ) );
            sched_job_add( sched, (void*)graphstate_create_chset( graphstate, graph_setForbidden( g, mina, minb ) ) );
        } else {
            if( sched_setBest( sched, graphstate, (void**)&tmp ) ) {
                if( tmp != NULL ) {
                    graphstate_decref( tmp );
                }
                graphstate_incref( graphstate );
            }
        }


        fmem_free( mergecost );

    } else {

        DBGPRINT( 11, "Worse than best" );
    }

    graphstate_unlock( graphstate );
    graphstate_decref( graphstate );
}

void alg_2k_job_free( sched_t *sched, void *job ) {
    graphstate_decref( (graphstate_t*)job );
}

int alg_2k_job_compare( sched_t *sched, void *joba, void *jobb ) {
    graphstate_t *gsa = (graphstate_t*)joba;
    graphstate_t *gsb = (graphstate_t*)jobb;
    ASSERT( gsa->cost >= 0 );
    ASSERT( gsb->cost >= 0 );
    return gsa->cost - gsb->cost;
}

void alg_2k_inc_limit_job( sched_t *sched, void *job ) {
    graphstate_t *gs = (graphstate_t*)job;
    gs->cost_left += 2;
}

double alg_2k_calcbranch( graph_cost_t ac, graph_cost_t bc ) {
    double max, min;
    double z, y, d, a, b;
    double powzmax, powzmin;
    a = ac/2.0;
    b = bc/2.0;

    if( a > b ) {
        max = a;
        min = a-b;
    } else {
        max = b;
        min = b-a;
    }

    z = 2.0;

    do {
        powzmax = pow( z, max );
        powzmin = pow( z, min );
        d = powzmax*max/z - powzmin*min/z;
        y = powzmax - powzmin - 1;
        d = y/d;
        z -= d;
    } while( d*d > 0.00001 );

    return z;
}
