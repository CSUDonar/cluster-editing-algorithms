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
#include "sched.h"
#include "fmem.h"
#include "strategy_best_first.h"

/* TODO: Documentation. Implemented using a minheap */

/* Internal storage */

typedef struct strategy_bestFirst_t {
    /* buffer:
     * A pointer to a vector of jobs (job = void*)
     * arranged as a min-heap
     */
    void **buffer;
    /* count: Number of jobs in the vector */
    size_t count;
    /* count: The size of the vector */
    size_t bufsize;
} strategy_bestFirst_t;


/* Local functions */

void strategy_bestFirst_create( sched_t *sched );
void strategy_bestFirst_free( sched_t *sched );
void strategy_bestFirst_job_add( sched_t *sched, void *job );
void *strategy_bestFirst_job_fetch( sched_t *sched );


/* Library interface */

const sched_strategy_t strategy_bestFirst = {
    strategy_bestFirst_create,
    strategy_bestFirst_free,

    strategy_bestFirst_job_add,
    strategy_bestFirst_job_fetch
};

/* Functions */

void strategy_bestFirst_create( sched_t *sched ) {
    strategy_bestFirst_t *s;
    s = fmem_alloc( sizeof( strategy_bestFirst_t ) );

    /* Initialize the structure. No jobs, space for 16 */
    s->count = 0;
    s->bufsize = 16;
    s->buffer = malloc( sizeof( void* ) * s->bufsize ); /* FIXME: mem */

    sched->strategy_storage = (void *)s;
}

void strategy_bestFirst_free( sched_t *sched ) {
    strategy_bestFirst_t *s;
    size_t i;
    s = (strategy_bestFirst_t *)(sched->strategy_storage);
    if( s == NULL ) {
        return;
    }

    /* Free all jobs in the vector */
    for( i=0; i<s->count; i++ ) {
        sched_job_free( sched, s->buffer[i] );
    }
    free( s->buffer ); /* FIXME: mem */

    fmem_free( s );
}

/* TODO: Return status code? */
void strategy_bestFirst_job_add( sched_t *sched, void *job ) {
    strategy_bestFirst_t *s;
    size_t i,j;
    void **newbuf;
    void *tmp;
    s = (strategy_bestFirst_t *)(sched->strategy_storage);

    /* If no place is left in the vector for a
     * new job, expand it to the dubble size
     */
    if( s->bufsize <= s->count ) {
        s->bufsize *= 2;
        newbuf = realloc( s->buffer, s->bufsize * sizeof( void* ) ); /* FIXME: mem */
        if( newbuf ) {
            s->buffer = newbuf;
        } else {
            /* TODO: Errorhandling, for now: revert and drop job */
            s->bufsize /= 2;
            sched_job_free( sched, job );
            return;
        }
    }

    /* Add the job at the end... */
    i = s->count;
    s->count++;
    s->buffer[i] = job;

    /* ...and reserve the heap property; bubble up. */
    while( i > 0 ) { /* TODO: do stop earlier when possible */
        j = (i-1)/2;
        if( 0 > sched_job_compare( sched, s->buffer[i], s->buffer[j] ) ) {
            tmp          = s->buffer[j];
            s->buffer[j] = s->buffer[i];
            s->buffer[i] = tmp;
        }
        i = j;
    }
}

void *strategy_bestFirst_job_fetch( sched_t *sched ) {
    strategy_bestFirst_t *s;
    void *job;
    void *tmp;
    size_t i,j;
    s = (strategy_bestFirst_t *)(sched->strategy_storage);

    if( s->count == 0 ) {
        return NULL;
    }

    /* Take the job at the beginning... */
    job = s->buffer[ 0 ];

    /* ...put the last job there... */
    s->buffer[ 0 ] = s->buffer[ s->count - 1 ];
    s->count--;

    /* ...and reserve the heap property; bubble down */
    i = 0;
    j = 1; /* TODO: make nicer, this is the first child node. */
    while( j < s->count ) {
        /* Which child node is smallest? */
        if( j+1 < s->count ) {
            if( 0 < sched_job_compare( sched, s->buffer[ j ], s->buffer[ j+1 ] ) ) {
                j++;
            }
        }
        /* Compare against that */
        if( 0 < sched_job_compare( sched, s->buffer[i], s->buffer[j] ) ) {
            tmp          = s->buffer[j];
            s->buffer[j] = s->buffer[i];
            s->buffer[i] = tmp;
        }
        i = j;
        /* Calculate next child node */
        j = ( i * 2 ) + 1;
    }
    return job;
}
