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
#include "sched.h"
#include "strategy_depth_first.h"
#include "fmem.h"

/* Depth first algorithm is implemented using a single linked list */

/* Internal storage */

/* Job placeholder */
typedef struct strategy_depthFirst_list_t {
    struct strategy_depthFirst_list_t *next;
    void *job;
} strategy_depthFirst_list_t;

/* Main structure */
typedef struct strategy_depthFirst_t {
    struct strategy_depthFirst_list_t *stack;
} strategy_depthFirst_t;


/* Local functions */

void strategy_depthFirst_create( sched_t *sched );
void strategy_depthFirst_free( sched_t *sched );
void strategy_depthFirst_job_add( sched_t *sched, void *job );
void *strategy_depthFirst_job_fetch( sched_t *sched );


/* Library interface */

const sched_strategy_t strategy_depthFirst = {
    strategy_depthFirst_create,
    strategy_depthFirst_free,

    strategy_depthFirst_job_add,
    strategy_depthFirst_job_fetch
};

/* Functions */

void strategy_depthFirst_create( sched_t *sched ) {
    strategy_depthFirst_t *s;
    s = fmem_alloc( sizeof( strategy_depthFirst_t ) );

    /* Empty stack in the beginnning */
    s->stack = NULL;

    sched->strategy_storage = (void *)s;
}

void strategy_depthFirst_free( sched_t *sched ) {
    strategy_depthFirst_t *s;
    strategy_depthFirst_list_t *cur;
    s = (strategy_depthFirst_t *)(sched->strategy_storage);

    if( s == NULL ) return;

    /* Iterate through stack and free jobs */
    while( (cur = s->stack) != NULL ) {
        s->stack = cur->next;
        sched_job_free( sched, cur->job );
        fmem_free( cur );
    }

    fmem_free( s );
}

/* TODO: Return status code? */
void strategy_depthFirst_job_add( sched_t *sched, void *job ) {
    strategy_depthFirst_t *s;
    strategy_depthFirst_list_t *cur;
    s = (strategy_depthFirst_t *)(sched->strategy_storage);

    /* Create a link */
    cur = fmem_alloc( sizeof( strategy_depthFirst_list_t ) );
    if( cur == NULL ) return;

    /* Put it in the beginning */
    cur->next = s->stack;
    cur->job = job;
    s->stack = cur;
}

void *strategy_depthFirst_job_fetch( sched_t *sched ) {
    strategy_depthFirst_t *s;
    strategy_depthFirst_list_t *cur;
    void *job;
    s = (strategy_depthFirst_t *)(sched->strategy_storage);

    cur = s->stack;
    /* Empty stack? */
    if( cur == NULL ) {
        return NULL;
    }
    /* Step forward in the stack */
    s->stack = cur->next;

    /* Pick the job and free the link */
    job = cur->job;
    fmem_free( cur );
    return job;
}
