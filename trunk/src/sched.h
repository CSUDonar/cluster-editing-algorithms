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
#ifndef SCHED_H
#define SCHED_H

typedef struct sched_t sched_t;

typedef struct sched_strategy_t {
    /* Scheduler, save the storage to sched->strategy_storage */
    void (*storage_create)( sched_t * );
    /* Scheduler storage */
    void (*storage_free)( sched_t * );

    /* Scheduler, job pointer */
    void (*job_add)( sched_t *, void * );
    /* Scheduler  */
    void *(*job_fetch)( sched_t* );
} sched_strategy_t;

typedef struct sched_algorithm_t {
    /* Scheduler, job pointer */
    void (*calculate)( sched_t*, void * );
    void (*job_free)( sched_t*, void * );
    /* TODO: Change interface to handle cost-objects as discussed? */
    int (*job_compare)( sched_t*, void *, void * );
    void (*inc_limit_job)( sched_t*, void * );
} sched_algorithm_t;

struct sched_t {
    const sched_strategy_t *strategy;
    const sched_algorithm_t *algorithm;

    void *strategy_storage;

    /* Best solution, FIXME: move to algorithm, maybe */
    void *best;
};


sched_t *   sched_create(       const sched_strategy_t *strategy,
                                const sched_algorithm_t *algorithm );

void        sched_free(         sched_t *sched );

void        sched_job_add(      sched_t *sched, void *job );
void        sched_job_free(     sched_t *sched, void *job );
int         sched_job_compare(  sched_t *sched, void *joba, void *jobb );

int         sched_stepone(      sched_t *sched );


void *      sched_getBest(      sched_t *sched );
/* returns if updated, and last value in *laststore, if laststore is not NULL */
int         sched_setBest(      sched_t *sched, void *best, void **laststore );
/* returns last best state */
void *      sched_resetBest( sched_t *sched );


void        sched_inc_limit_job( sched_t *sched, void *job );


#endif
