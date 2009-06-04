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
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "debug.h"
#include "graph.h"
#include "graphstate.h"
#include "sched.h"
#include "strategy_depth_first.h"
#include "alg_3k.h"
#include "alg_2k.h"
#include "alg_2_62k.h"
#include "postprocess.h"
#include "fmem.h"

#include "datasource_random.h"
#include "datasource_file.h"

#ifdef OPENCV_COIN
#include "datasource_cv_coin.h"
#include "datasource_cv_coin_anim.h"
#endif

typedef struct algorithm_list_t {
    char *name;
    sched_algorithm_t *alg;
} algorithm_list_t;

algorithm_list_t alg_list[] = {
    {"2k",      &alg_2k},
    {"2.62k",   &alg_2_62k},
    {"3k",      &alg_3k},
    {NULL, NULL}
};


void usage( char *cmd );

void usage( char *cmd ) {
    fprintf( stderr,
            "Usage: %s <options>\n"
            "\n"
            "Options:\n"
#if DEBUG
            "    -d <num>      : Set debug level\n"
#endif
            "    -s <num>      : Seed random number generator\n"
            "    -a <algorithm>: Select algorithm\n"
            "    -h            : Show this help message\n"
            "\n", cmd);

    fprintf( stderr,
            "  Loading files:\n"
            "    -f <filename> : Read cluster file\n"
            "    -r <num of nodes>:<num of cliques>:<noise>:<max weight>\n"
            "                  : Generate random graph\n"
#ifdef OPENCV_COIN
            "    -c <image>    : Filename of image with coins, (can be camera)\n"
            "    -n <image>    : Filename of image with coins, (can be camera)\n"
#endif
            "\n"
            "  Example:\n"
            "    %s -r 100:10:7:3\n"
            , cmd );
    exit(1);
}


int main( int argc, char *argv[] ) {
    graph_t *graph;
    graphstate_t *initstate;
    graphstate_t *beststate;

    graph_index_t *cliqueid;
    sched_t *sched;
    time_t seed;
    char *alg_name = NULL;
    sched_algorithm_t *alg = NULL;

    const datasource_t *datasource = NULL;
    datasource_storage_t *ds_store;

#if DEBUG
    long iterationcount;
#endif

    char *ds_args = NULL;

    int opt,i;

    /* FIXME: use better seed than time() */
    seed = time(NULL);

    /* Parse command line arguments */
    while( (opt = getopt( argc, argv,
#if DEBUG
                    "d:"
#endif
                    "s:a:hf:r:c:n:" ) ) != -1 ) {
        switch( opt ) {
#if DEBUG
            case 'd':
                g_debug_level = atoi( optarg );
                break;
#endif
            case 's': seed = atoi( optarg ); break;
            case 'a': alg_name = optarg; break;
            case 'f':
                      if( datasource != NULL ) usage( argv[0] );
                      datasource = &datasource_file;
                      ds_args = optarg;
                      break;
            case 'r':
                      if( datasource != NULL ) usage( argv[0] );
                      datasource = &datasource_random;
                      ds_args = optarg;
                      break;
#ifdef OPENCV_COIN
            case 'c':
                      if( datasource != NULL ) usage( argv[0] );
                      datasource = &datasource_cv_coin;
                      ds_args = optarg;
                      break;
            case 'n':
                      if( datasource != NULL ) usage( argv[0] );
                      datasource = &datasource_cv_coin_anim;
                      ds_args = optarg;
                      break;
#endif
            case 'h':
            default:
                usage( argv[0] );
        }
    }

    if( datasource == NULL ) {
        usage( argv[0] );
    }

    if( ds_args == NULL ) {
        ds_args = "";
    }

    if( alg_name == NULL ) {
        alg_name = alg_list[0].name;
        alg      = alg_list[0].alg;
    } else {
        for( i=0; alg_list[i].name != NULL; i++ ) {
            if( strcmp( alg_list[i].name, alg_name ) == 0 ) {
                alg = alg_list[i].alg;
                break;
            }
        }
    }

    if( alg == NULL ) {
        fprintf( stderr, "Unknown algorithm: %s\nAlgoritms avalible:\n", alg_name );
        for( i=0; alg_list[i].name != NULL; i++ ) {
            fprintf( stderr, "  %s\n", alg_list[i].name );
        }
        usage( argv[0] );
    }

    DBGLONG( 2, seed );
    srand( seed );

    ds_store = datasource_create( datasource, ds_args );

    /* Create sheduler (reuse every frame) */
    sched = sched_create( &strategy_depthFirst, alg );

    /* Start loop, (runs once for datasource random and file,
     * continuous for cv/camera
     */
    while( ( graph = datasource_get( ds_store ) ) != NULL ) {
        DBGPRINT( 9, "New frame" );

        /* First reference is treated as local reference */
        ASSERT( graph );
        if( graph_getNodeCount( graph ) > 0 ) {
            initstate = graphstate_create_base( graph, 0 );

#if DEBUG
            iterationcount = 0;
#endif
            for(;;) {
                DBGPRINT( 10, "-------------- Incrementing k-limit" );
                /* Create a reference for job queue */
                graphstate_incref( initstate );
                sched_job_add( sched, initstate );
                DBGLONG( 10, initstate->cost_left );
                while( sched_stepone( sched ) ) {
                    DBGPRINT( 15, "step done" );
#if DEBUG
                    iterationcount++;
#endif
                }
                DBGLONG( 10, iterationcount );

                beststate = sched_getBest( sched );
                if( beststate == NULL ) {
                    graphstate_lock( initstate, 0, 0 ); /* basecase: already got a cost */
                    sched_inc_limit_job( sched, initstate );
                    graphstate_unlock( initstate );
                } else {
                    /* We found something */
                    break;
                }
            }
            DBGLONG( 5, iterationcount );

            beststate = sched_resetBest( sched );

            DBGINT( 5, beststate->type );
            /* print best state */
            /* node already visited through algorithm: already got a cost */
            graphstate_lock( beststate, 0, 0 );


            /* same graph everywhare, therefore accessed through graph */
            DBGLONG( 1, beststate->cost );
            printf( "Best cost: %ld\n", beststate->cost );
        }
        cliqueid = postprocess_enumerate_cliques( graph, initstate );
        datasource_show( ds_store, graph, cliqueid );
        fmem_free( cliqueid );

        graphstate_unlock( beststate );
        graphstate_decref( beststate );

        /*remove local reference to initstate*/
        graphstate_decref( initstate );

        graph_free( graph );


    }

    /* Free data structures */
    sched_free( sched );

    /* End loop */
    datasource_free( ds_store );

    return 0;
}
