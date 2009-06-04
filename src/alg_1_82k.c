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
#include "alg_1_82k.h"
#include "fmem.h"
#include "string.h"
#include "graphstate.h"
#include "graph.h"

void alg_1_82k_calculate( sched_t *sched, void *job );
void alg_1_82k_job_free( sched_t *sched, void *job );
int alg_1_82k_job_compare( sched_t *sched, void *joba, void *jobb );
void alg_1_82k_inc_limit_job( sched_t *sched, void *job );
double alg_1_82k_calcbranch( graph_cost_t ac, graph_cost_t bc );

sched_algorithm_t alg_1_82k = {
    alg_1_82k_calculate,
    alg_1_82k_job_free,
    alg_1_82k_job_compare,
    alg_1_82k_inc_limit_job
};

void alg_1_82k_calculate( sched_t *sched, void *job ) {
    graphstate_t *graphstate = (graphstate_t *) (graphstate_t*)job;
    graphstate_t *tmp;
    graph_t *g;
    graph_index_t a,b,c,d,x,y,z,v1,v2,tempb,val, cost, best_cost; /* TODO: Not all of these are needed */
    graph_cost_t ca, cb;
    graph_cost_t cmerge, cforbid;
    graph_size_t maxnodeindex, nodecount, edgecount;
    graph_cost_t *mergecost;
    double branchvec;
    double branchvecmin;
    graph_index_t mina, minb;

    int change;
    int i;
    int branchingdone;
    int nodes, edges;
    int edge_setup;
    int edge;
    int edge_setup_opt, cost_opt;
    graph_chSet_t *chs[6];
    graph_chSet_t *chs_opt[6];



    tmp = (graphstate_t *)sched_getBest( sched );
    graphstate_lock( graphstate, 2, 1 );
    if( graphstate->cost_left < 0 ) { /* If no cost is left, leave it */
        graphstate_unlock( graphstate );
        graphstate_decref( graphstate );
        return;
    }

    if( tmp == NULL || graphstate->cost < tmp->cost ) {

        DBGPRINT( 15, "New job" );

        g = graphstate->c.org->graph;
        maxnodeindex = graph_getNodeCount( g ) - 1;

        /* TODO: no reallocation */
        mergecost = (graph_cost_t) fmem_alloc_arr( sizeof( graph_cost_t ), graph_getEdgeCount( g ) );

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
                                 * Two edges is zero-edges = both edges is resolved, one new zero-edge is created
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

                            branchvec = alg_1_82k_calcbranch( cmerge, cforbid );

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

        if( mina >= 0 && branchvecmin < 1.76 ) { /* If Branch Rule A */
            sched_job_add( sched, (void*)graphstate_create_chset( graphstate, graph_merge( g, mina, minb ) ) );
            sched_job_add( sched, (void*)graphstate_create_chset( graphstate, graph_setForbidden( g, mina, minb ) ) );
        /*
         TODO: The nested B branching rules test is in severe need of cleanup...
         TODO: Apparently the edge with the best branching number is not needed in the B branching scenario?
         */
        } else {
        	branchingdone = 0;
            /* Test branching rules B11-B44 for all edges in O(n^4) total*/
            /* Find all triangles (x,y,z) in O(n^3) total */
            for ( x = 0; branchingdone < 1 && x >= 0; x = graph_getNext( g, x ) ) {
            	for ( y = graph_getNext( g, x ); branchingdone < 1 && y >= 0; y = graph_getNext( g, y) ) {
            		if ( graph_getValue( g, x, y ) > 0 ) {
            			for( z = graph_getNext( g, y ); branchingdone < 1 && z >= 0; z = graph_getNext( g, z ) ) {
            				if ( graph_getValue( g, z, x ) > 0 && graph_getValue( g, z, y ) > 0 ) {
            					/* Find both v1 & v2 in O(n) total */
								for ( v1 = 0; branchingdone < 1 && v1 >= 0; v1 = graph_getNext( g, v1 ) ) {
                                    /* if v1 is Branch B legal and is not x,y,z */
                                    if ( v1 != z && v1 != x && v1 != y &&
                                                ( ( graph_getValue( g, v1, x ) > 0 && graph_getValue( g, v1, y ) < 0 ) ||
                                                  ( graph_getValue( g, v1, x ) == 0 && graph_getValue( g, v1, y ) == 0 ) ||
                                                  ( graph_getValue( g, v1, x ) == 0 && graph_getValue( g, v1, y ) < 0  && graph_getValue( g, v1, z ) >= 0 ) ||
                                                  ( graph_getValue( g, v1, x ) > 0 && graph_getValue( g, v1, y ) == 0  && graph_getValue( g, v1, z ) <= 0 ) ) ) {
                                        for ( v2 = graph_getNext( g, v1); branchingdone < 1 && v2 >= 0; v2 = graph_getNext( g, v2 ) ) {
                                            /* if v2 is Branch B legal and is not x,y,z,v1 */
                                            if ( v2 != v1 && v2 != z && v2 != x && v2 != y &&
                                                         ( ( graph_getValue( g, v2, x ) > 0 && graph_getValue( g, v2, y ) < 0 ) ||
                                                           ( graph_getValue( g, v2, x ) == 0 && graph_getValue( g, v2, y ) == 0 ) ||
                                                           ( graph_getValue( g, v2, x ) == 0 && graph_getValue( g, v2, y ) < 0  && graph_getValue( g, v2, z ) >= 0 ) ||
                                                           ( graph_getValue( g, v2, x ) > 0 && graph_getValue( g, v2, y ) == 0  && graph_getValue( g, v2, z ) <= 0 ) ) ) {
                                                /* Branch on (x,y) */
                                                sched_job_add( sched, (void*)graphstate_create_chset( graphstate, graph_merge( g, x, y ) ) );
                                                sched_job_add( sched, (void*)graphstate_create_chset( graphstate, graph_setForbidden( g, x, y ) ) );
                                                branchingdone = 1;
                                                break;
                                            }
                                        }
                                        if ( branchingdone == 0 ) {
                                            break; /* There aren't two Branch B legal edges to the current a and b nodes */
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if ( x == maxnodeindex && y == maxnodeindex  && branchingdone == 0 ) {
                        /* TODO: No edge fits under either Branch Rule A or B -> Use alternative solutions */

                        /*TODO: replace node and edge count with this code and scrap edge and node vars in graph */
                        /*traverse to get node count*/

                        /* edge count */
                        edges=0;
                        nodes=0;

                        for(a = 0; a >= 0; a=graph_getNext(g, a) ){
                            nodes++;
                            for(b=graph_getNext(g, a); b >= 0; b=graph_getNext(g, b) ){
                                if ( graph_getValue(g, a, b) ) {
                                    edges++;
                                }
                                else {
                                    c = a;
                                    d = b;
                                }
                            }
                        }

                        if ( edges == ( ( nodes * ( nodes - 1 ) )^2 ) / 2 ) {
			                /* Remaining graph is a clique. We are done.*/
                        } else if ( nodes <= 4 ) {
                        /* Remaining graph is small. Solve with Brute force */
                        /*this is by no means concidered working code, even though it compiles, it has to be properly integrated*/
                            cost_opt = 0;
                            edge_setup_opt = 0x0;
                            edge_setup = 0x000000;
                            edge = 0x1;

                            while (edge_setup <= 0x111111) {
                                for (i = 0; i < 6; i++)
                                    chs[i]=NULL;

                                i = 0;
                                cost = 0;
                                a = 0;
                                b = graph_getNext(g, a);
                                while ( a >= 0 ) {
                                    while( b >= 0 ) {

                                        val = graph_getValue(g, a, b);
                                        if ( val && !(edge_setup & edge) )
                                            chs[i] = graph_setForbidden(g, a, b);
                                        if ( !val && (edge_setup & edge) )
                                            chs[i] = graph_setPersistant(g, a, b);
                                        if (chs[i] != NULL)
                                            cost = graph_apply(g, chs[i], 2, 1);

                                        i++;
                                        edge << 1;
                                        b=graph_getNext(g, b);
                                    }
                                   a=graph_getNext(g, a);
                                }

                                if (cost < cost_opt && graph_isClusterGraph(g) ){
                                    cost_opt = cost;
                                    memcpy( chs_opt, chs, 7);
                                }
                                edge_setup++;
                                /* step back to were we were at the start */
                                graphstate_lock( graphstate, 2, 1);
                            }
                            
                            for( i=0; i < 6; i++){
                                graph_apply(g, chs[i], 2, 1);
                            }

                        } else if ( edges - 1 == ( ( nodes * ( nodes - 1 ) )^2 ) / 2 ) {

             /*Remaining graph is a clique missing an edge. Solve with min-cut.*/
                        } else {
			  /*Remaining graph is a circle or a path. Solve with Dynamic Programming.*/
                        }
                    }
                }
            }
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

void alg_1_82k_job_free( sched_t *sched, void *job ) {
    graphstate_decref( (graphstate_t*)job );
}

int alg_1_82k_job_compare( sched_t *sched, void *joba, void *jobb ) {
    graphstate_t *gsa = (graphstate_t*)joba;
    graphstate_t *gsb = (graphstate_t*)jobb;
    ASSERT( gsa->cost >= 0 );
    ASSERT( gsb->cost >= 0 );
    return gsa->cost - gsb->cost;
}

void alg_1_82k_inc_limit_job( sched_t *sched, void *job ) {
    graphstate_t *gs = (graphstate_t*)job;
    gs->cost_left += 2;
}

double alg_1_82k_calcbranch( graph_cost_t ac, graph_cost_t bc ) {
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
