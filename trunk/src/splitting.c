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
#include "splitting.h"


/* TODO: use getCost or getValue? */
splitting_t *splitting_split(graph_t *graph) {
    graph_index_t pos = 0, kernelCount = 0, i, j, k;
    graph_index_t nodes = graph_getNodeCount(graph);
    graph_index_t *kernels = malloc(sizeof(int) * (nodes + 1));
    graph_index_t *order = malloc(sizeof(graph_index_t) * nodes);
    splitting_t *collection = malloc(sizeof(splitting_t));

    for (i = 0; i < nodes; i++) {
        order[i] = i;
    }
    for (i = 0; i < nodes; i++) {
        if (i == pos) {
            kernels[kernelCount++] = pos++;
        }
        for (j = pos; j < nodes; j++) {
            if (graph_getValue(graph, order[i], order[j]) > 0) {
                graph_index_t tmp = order[j];
                order[j] = order[pos];
                order[pos] = tmp;
                pos++;
            }
        }
    }
    kernels[kernelCount] = nodes; /* to avoid special cases later */

    collection->graphCount = kernelCount;
    collection->idxMap = malloc(sizeof(splitting_idx_t) * nodes);
    collection->graphs = malloc(sizeof(graph_t*) * kernelCount);

    for (i = 0; i < kernelCount; i++) {
        graph_index_t kernelSize = kernels[i + 1] - kernels[i];
        graph_t *subgraph = collection->graphs[i] = graph_create(kernelSize);
        for (j = 0; j < kernelSize; j++) {
            splitting_idx_t * idx =
                &collection->idxMap[order[j + kernels[i]]];
            idx->subGraph = i;
            idx->idx = j;

            for (k = j + 1; k < kernelSize; k++) {
                graph_setValue(subgraph, j, k, graph_getValue(graph, order[j+kernels[i]], order[k+kernels[i]]));
            }
        }
    }
    free(kernels);
    free(order);
    return collection;
}

void splitting_free(splitting_t *graphSet) {
    graph_index_t i;
    for (i = 0; i < graphSet->graphCount; i++) {
        graph_free(graphSet->graphs[i]);
    }
    free(graphSet->graphs);
    free(graphSet->idxMap);
    free(graphSet);
}

int splitting_isEdge(splitting_t *graphSet, graph_index_t node1,
                     graph_index_t node2) {
    graph_index_t sg1 = graphSet->idxMap[node1].subGraph;
    graph_index_t sg2 = graphSet->idxMap[node2].subGraph;
    graph_index_t idx1 = graphSet->idxMap[node1].idx;
    graph_index_t idx2 = graphSet->idxMap[node2].idx;
    /* TODO: add a graph_isEdge */
    return sg1 == sg2 && graph_getValue(graphSet->graphs[sg1], idx1, idx2) > 0;
}
