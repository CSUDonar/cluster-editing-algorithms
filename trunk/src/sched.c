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
#include "fmem.h"

sched_t *sched_create(  const sched_strategy_t *strategy,
                        const sched_algorithm_t *algorithm
                     ) {
    sched_t *sched = fmem_alloc( sizeof( sched_t ) );
    if( sched == NULL ) return NULL;

    sched->strategy = strategy;
    sched->algorithm = algorithm;
    sched->strategy_storage = NULL;

    (*sched->strategy->storage_create)( sched );

    if( sched->strategy_storage == NULL ) {
        sched_free( sched );
        return NULL;
    }

    sched->best = NULL;

    return sched;
}

void sched_free( sched_t *sched ) {
    if( sched->strategy_storage )
        (*sched->strategy->storage_free)( sched );
    free( sched );
}

void sched_job_add( sched_t *sched, void *job ) {
    (*sched->strategy->job_add)( sched, job );
}

void sched_job_free( sched_t *sched, void *job ) {
    (*sched->algorithm->job_free)( sched, job );
}

int sched_job_compare( sched_t *sched, void *joba, void *jobb ) {
    return (*sched->algorithm->job_compare)( sched, joba, jobb );
}

int sched_stepone(  sched_t *sched ) {
    void *job = (*sched->strategy->job_fetch)( sched );
    if( job != NULL ) {
        (*sched->algorithm->calculate)( sched, job );
        return 1;
    }
    return 0;
}


void *sched_getBest( sched_t *sched ) {
    return sched->best;
}

int sched_setBest( sched_t *sched, void *best, void **laststore ) {
    void *last_best = sched->best;
    if( laststore != NULL ) {
        *laststore = last_best;
    }
    if( last_best == NULL ) {
        sched->best = best;
        return 1;
    } else if( sched_job_compare( sched, sched->best, best ) >= 0 ) {
        sched->best = best;
        return 1;
    }
    return 0;
}

void *sched_resetBest( sched_t *sched ) {
    void *last_best = sched->best;
    sched->best = NULL;
    return last_best;
}

void        sched_inc_limit_job( sched_t *sched, void *job ) {
    (*sched->algorithm->inc_limit_job)( sched, job );
}
