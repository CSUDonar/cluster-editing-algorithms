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
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "graphfile.h"

int graphfile_parseline( char *line, int *elems, int length );



int graphfile_parseline( char *line, int *elems, int length ) {
    char *tok;
    int i;

    /* TODO: strtok isn't thread safe */
    tok = strtok( line, " \t\n" );

    for( i=0; i<length && tok; i++ ) {
        elems[i] = atoi( tok );
        tok = strtok( NULL, " \t\n" );
    }
    return i;
}


graph_t *graphfile_readfobj( FILE *fp ) {
    char line[1024];
    int elems[32];
    int cnt, nodecnt;
    int i, j;

    graph_t *graph = NULL;

    /* Fetch header */
    if( !fgets( line, 1024, fp ) ) {
        return NULL;
    }

    /* Parse header */
    cnt = graphfile_parseline( line, elems, 32 );

    if( cnt == 1 ) {
        nodecnt = elems[0];
        graph = graph_create( nodecnt );
    } else {
        DBGSTR( 0, line );
        return NULL;
    }

    /* Update all edges to be non-edges, needed for unweighted files */

    /* TODO: better way to implement this? */
    for( i=0; i<nodecnt; i++ ) {
        for( j=i+1; j<nodecnt; j++ ) {
            graph_setValue( graph, i, j, -1 );
        }
    }

    /* Fetch rest */
    while( NULL != fgets( line, 1024, fp ) ) {
        /* Parse rest */
        cnt = graphfile_parseline( line, elems, 32 );
        if( cnt == 2 ) {
            graph_setValue( graph, elems[0], elems[1], 1 );
        } else if( cnt == 3 ) {
            graph_setValue( graph, elems[0], elems[1], elems[2] );
        } else {
            DBGPRINT( 0, "Unknown line" );
            DBGSTR( 0, line );
        }
    }

    return graph;
}


graph_t *graphfile_readfile( const char *filename ) {
    FILE *fp;
    graph_t *graph;
    fp = fopen( filename, "r" );
    if( fp == NULL ) {
        return NULL;
    }

    graph = graphfile_readfobj( fp );

    fclose( fp );
    return graph;
}




int graphfile_writefobj( const graph_t *graph, FILE *fp ) {
    int i, j, nodecnt;
    nodecnt = graph_getNodeCount( graph );
    fprintf( fp, "%d\n", nodecnt );
    for( i=0; i<nodecnt; i++ ) {
        for( j=i+1; j<nodecnt; j++ ) {
            fprintf( fp, "%d %d %ld\n", i, j, graph_getValue( graph, i, j ) );
        }
    }
    return 1;
}

int graphfile_writefile( const graph_t *graph, const char *filename ) {
    int retval;
    FILE *fp;
    fp = fopen( filename, "w" );
    if( fp == NULL ) {
        return 0;
    }

    retval = graphfile_writefobj( graph, fp );

    fclose( fp );
    return retval;
}
