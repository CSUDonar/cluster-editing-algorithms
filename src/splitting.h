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
#ifndef SPLITTING_H
#define SPLITTING_H

#include "graph.h"

typedef struct {
  graph_index_t subGraph;
  graph_index_t idx;
} splitting_idx_t;

typedef struct {
  graph_index_t graphCount;
  graph_t **graphs;
  splitting_idx_t *idxMap;
} splitting_t;

splitting_t *splitting_split(graph_t *graph);

void splitting_free(splitting_t *graphSet);

int splitting_isEdge(splitting_t *graphSet, graph_index_t node1, graph_index_t node2);

#endif
