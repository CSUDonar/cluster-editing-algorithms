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
#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "graph.h"
#include "graphstate.h"

typedef struct datasource_t {
    void *(*create)( char * ); /* Args as string */
    void (*free)( void * ); /* Storage */
    graph_t *(*get)( void * ); /* Storage */
    void (*show)( void *, graph_t *, graph_index_t * ); /* Storage, Graph, Cliqueids */
} datasource_t;

typedef struct datasource_storage_t {
    const datasource_t *datasource;
    void *storage;
} datasource_storage_t;

datasource_storage_t *datasource_create( const datasource_t *source, char *args );
void datasource_free( datasource_storage_t *ds );

graph_t *datasource_get( datasource_storage_t *ds );
void datasource_show( datasource_storage_t *ds, graph_t *graph, graph_index_t *cliqueid );

#endif
