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
#include "graph.h"
#include "graphstate.h"
#include "debug.h"
#include "kernel.h"

typedef struct {
    graph_index_t node;
    graph_value_t cost;
} kernel_heapnode_t;


typedef struct {
    graph_index_t node;
    kernel_heapnode_t *heap;
    graph_index_t *mapping;
    graph_size_t size;
} kernel_heap_t;

void restoreProp(kernel_heap_t *heaps, graph_index_t node1, graph_index_t node2, int forbOrperm);
kernel_heapnode_t *findheapnode(kernel_heap_t *heaps, graph_index_t node1, graph_index_t node2, int forbOrperm);
void getMax(kernel_heap_t *heaps, graph_value_t *maxIcp, graph_value_t *maxIcf, kernel_heap_t *bestIcp, kernel_heap_t *bestIcf, graph_index_t nodes);
#if DEBUG
void printheaps(kernel_heap_t *heaps, graph_index_t nodes);
#endif

graphstate_t *kernel_kernelize(graphstate_t *gs, graph_cost_t fixpoint, graph_cost_t bookkeepingValue) {
    
    graph_t *graph;
    graphstate_t *newgs;
    graph_cost_t kparam = 0;
    graph_index_t i = 0, j = 0, k, node1, node2, node3;
    graph_index_t *lookup;
    graph_value_t cost1,cost2, cost3, maxIcp = 0, maxIcf = 0;
    graph_size_t nodes = 0;
    kernel_heap_t bestIcp, bestIcf;
    kernel_heap_t *heaps;
    kernel_heapnode_t *heapnode, *heapnode1, *heapnode2;

   
    graphstate_lock( gs, fixpoint, bookkeepingValue );
    graph = gs->c.org->graph;
    ASSERT( graph );

    /* Have to count the existing nodes because merging doesnt update this */
    while(i >= 0) {
        nodes++;
        i = graph_getNext(graph, i);
    }
    
    lookup = malloc(sizeof(graph_index_t)*nodes);
    heaps = malloc(sizeof(kernel_heap_t)*nodes*2);
    
    i = 0;
    while(i >= 0) {
        lookup[j++] = i;
        i = graph_getNext(graph, i);
    }
    heaps[nodes-2].node = nodes - 1;
    heaps[nodes-1].node = nodes - 1;
    /* Calculate and initialize all icp and icf values on the heaps */
    for (i = 0; i < nodes*2-2; i += 2) {
        node1 = i/2;
        heaps[i].node = node1;
        heaps[i].size = 0;
        heaps[i].heap = malloc(sizeof(kernel_heapnode_t)*(nodes-node1-1));
        heaps[i].mapping = malloc(sizeof(graph_index_t)*(nodes-node1-1));
        heaps[i+1].node = node1;
        heaps[i+1].size = 0;
        heaps[i+1].heap = malloc(sizeof(kernel_heapnode_t)*(nodes-node1-1));
        heaps[i+1].mapping = malloc(sizeof(graph_index_t)*(nodes-node1-1));
        for (j = 0; j < nodes-node1-1; j++) {
            node2 = node1+j+1;
            heaps[i].mapping[j] = j;
            heaps[i+1].mapping[j] = j;
            heaps[i].heap[j].cost = 0;
            heaps[i+1].heap[j].cost = 0;
            heaps[i].heap[j].node = node2;
            heaps[i+1].heap[j].node = node2;


            for (k = 0; k < nodes; k++) {
                if (k == node1 || k == node2) {
                    continue;
                }
                cost1 = graph_getValue(graph, lookup[node1], lookup[k]);
                cost2 = graph_getValue(graph, lookup[node2], lookup[k]);
                if (cost1 > 0 && cost2 > 0) {
                    heaps[i].heap[j].cost += (cost1 < cost2 ? cost1:cost2);
                }
                else if ((cost1 > 0 && cost2 < 0) || (cost1 < 0 && cost2 > 0)) {
                    heaps[i+1].heap[j].cost += (abs(cost1) < abs(cost2) ? abs(cost1):abs(cost2));
                }
            }
            cost1 = graph_getValue(graph, lookup[node1], lookup[node2]);
            if (abs(cost1) > 50000L){
                heaps[i].heap[j].cost = 0;
                heaps[i+1].heap[j].cost = 0;
            }
            else if (cost1 > 0) {
                heaps[i].heap[j].cost += cost1;
            }
            else {
                heaps[i+1].heap[j].cost -= cost1;
            }
            restoreProp(heaps, node1, node2, 0);
            restoreProp(heaps, node1, node2, 1);
            heaps[i].size++;
            heaps[i+1].size++;


            if (heaps[i].heap[0].cost > maxIcf) {
                maxIcf = heaps[i].heap[0].cost;
                bestIcf = heaps[i];
            }
            if (heaps[i+1].heap[0].cost > maxIcp) {
                maxIcp = heaps[i+1].heap[0].cost;
                bestIcp = heaps[i+1];
            }
        }
    }

    graphstate_unlock( gs );

    /* Phase 2, find the maximum induced costs and merge or setForbidden accordingly */
    while ( 1 ) {
        graphstate_lock( gs, fixpoint, bookkeepingValue );
        graph = gs->c.org->graph;
        kparam = gs->cost_left; /* TODO: Use access method */

        DBGLONG( 11, kparam );
        DBGLONG( 11, maxIcp );
        DBGLONG( 11, maxIcf );

        if( (maxIcp*fixpoint <= kparam && maxIcf*fixpoint <= kparam) || kparam < 1 ) {
            graphstate_unlock( gs );
            break;
        }

        /* Forbid edge... */
        if (maxIcp*fixpoint > kparam) {
            node1 = bestIcp.node;
            node2 = bestIcp.heap[0].node;
            cost1 = graph_getValue(graph, lookup[node1], lookup[node2]);
            if (cost1 < -50000L) {
                bestIcp.heap[0].cost = 0;
                findheapnode(heaps, node1, node2, 0)->cost = 0;
                restoreProp(heaps, node1, node2, 0);
                restoreProp(heaps, node1, node2, 1);
                getMax(heaps, &maxIcp, &maxIcf, &bestIcp, &bestIcf, nodes);
                graphstate_unlock( gs );
                continue;
            }
            for(i = 0; i < nodes; i++) {
                node3 = i;
                if (heaps[i*2].node < 0 || node3 == node1 || node3 == node2) {
                    continue;
                }
                cost2 = graph_getValue(graph, lookup[node1], lookup[node3]);
                cost3 = graph_getValue(graph, lookup[node2], lookup[node3]);
                DBGLONG( 12, cost2 );
                DBGLONG( 12, cost3 );

                if (cost2 > 0 && cost1 > 0) {
                    heapnode = findheapnode(heaps, node2, node3, 1);
                    heapnode->cost += cost2;
                    restoreProp(heaps, node2, node3, 1);
                    heapnode = findheapnode(heaps, node2, node3, 0);
                    heapnode->cost -= cost1>cost2?cost2:cost1;
                    restoreProp(heaps, node2, node3, 0);
                }
                else if (cost1*cost2 < 0) {
                    heapnode = findheapnode(heaps, node2, node3, 1);
                    if(cost1 > 0) {
                        heapnode->cost -= cost1 > abs(cost2) ? abs(cost2):cost1;
                    }
                    else {
                        heapnode->cost += cost2 < abs(cost1) ? 0:cost2 - abs(cost1);
                    }
                    restoreProp(heaps, node2, node3, 1);
                }
                if (cost3 > 0 && cost1 > 0) {
                    heapnode = findheapnode(heaps, node1, node3, 1);
                    heapnode->cost += cost3;
                    restoreProp(heaps, node1, node3, 1);
                    heapnode = findheapnode(heaps, node1, node3, 0);
                    heapnode->cost -= cost1>cost3?cost3:cost1;
                    restoreProp(heaps, node1, node3, 0);
                }
                else if (cost1*cost3 < 0) {
                    heapnode = findheapnode(heaps, node1, node3, 1);
                    if(cost1 > 0) {
                        heapnode->cost -= cost1>abs(cost3)?abs(cost3):cost1;
                    }
                    else {
                        heapnode->cost += cost3<abs(cost1)?0:cost3 - abs(cost1);
                    }
                    restoreProp(heaps, node1, node3, 1);
                }
            }
            heapnode = findheapnode(heaps, node1, node2, 0);
            heapnode->cost = 0;
            restoreProp(heaps, node1, node2, 0);
            heapnode = findheapnode(heaps, node1, node2, 1);
            heapnode->cost = 0;
            restoreProp(heaps, node1, node2, 1);

            newgs = graphstate_create_chset( gs,
                graph_setForbidden(graph, lookup[node1], lookup[node2])
                );
        }

        /* Merge edge */
        else if (maxIcf*fixpoint > kparam) {
            node1 = bestIcf.node;
            node2 = bestIcf.heap[0].node;
            heaps[node2*2].node = -1;
            heaps[node2*2 + 1].node = -1;
            for(i = 0; i < nodes; i ++) {
                if (heaps[2*i].node < 0 || i == node2 || i == node1) {
                    continue;
                }
                heapnode1 = findheapnode(heaps, node2, i, 0);
                heapnode2 = findheapnode(heaps, node2, i, 1);
                heapnode1->cost = 0;
                restoreProp(heaps, node2, i, 0);
                heapnode2->cost = 0;
                restoreProp(heaps, node2, i, 1);

                heapnode1 = findheapnode(heaps, node1, i, 0);
                heapnode2 = findheapnode(heaps, node1, i, 1);
                heapnode1->cost = 0;
                heapnode2->cost = 0;
                for (j = 0; j < nodes; j++) {
                    if (heaps[2*j].node < 0 || j == node1 || j == node2 || j == i) {
                        continue;
                    }
                    
                    cost1 = graph_getValue(graph, lookup[node1], lookup[j]) + 
                        graph_getValue(graph, lookup[node2], lookup[j]);
                    cost2 = graph_getValue(graph, lookup[i], lookup[j]);
                    if (cost1 > 0 && cost2 > 0) {
                        heapnode1->cost += (cost1 < cost2 ? cost1:cost2);
                    }
                    else if ((cost1 > 0 && cost2 < 0) || (cost1 < 0 && cost2 > 0)) {
                        heapnode2->cost += (abs(cost1) < abs(cost2) ? abs(cost1):abs(cost2));
                    }
                }
                cost1 = graph_getValue(graph, lookup[node1], lookup[i]) + 
                    graph_getValue(graph, lookup[node2], lookup[i]);
                if (abs(cost1) > 50000L) {
                    findheapnode(heaps, node1, i, 0)->cost = 0;
                    findheapnode(heaps, node1, i, 1)->cost = 0;
                }
                else if (cost1 > 0) {
                    heapnode1->cost += cost1;
                }
                else {
                    heapnode2->cost -= cost1;
                }
                restoreProp(heaps, node1, i, 0);
                restoreProp(heaps, node1, i, 1);
            }

            heapnode = findheapnode(heaps, node1, node2, 0);
            heapnode->cost = 0;
            restoreProp(heaps, node1, node2, 0);
            heapnode = findheapnode(heaps, node1, node2, 1);
            heapnode->cost = 0;
            restoreProp(heaps, node1, node2, 1);
            
            newgs = graphstate_create_chset( gs,
                graph_merge(graph, lookup[node1], lookup[node2])
                );
        }
#if DEBUG
        else {
            DBGPRINT( 0, "Invalid case" );
            DBGLONG( 0, kparam );
            DBGLONG( 0, maxIcp );
            DBGLONG( 0, maxIcf );
            ASSERT( 0 );
        }
#endif

        getMax(heaps, &maxIcp, &maxIcf, &bestIcp, &bestIcf, nodes);
        graphstate_unlock( gs );
        graphstate_decref( gs );
        gs = newgs;
    }
    for (i = 0; i < nodes*2-2; i += 2) {
        free(heaps[i].heap);
        free(heaps[i+1].heap);
        free(heaps[i].mapping);
        free(heaps[i+1].mapping);
    }
    free(lookup);
    free(heaps);

   return gs;
}

#if DEBUG
void printheaps(kernel_heap_t *heaps, graph_index_t nodes) {
    graph_index_t i, j;
    for (i = 0; i < nodes-1; i++) {
        printf("node %ld\n", i);
        for (j = 0; j < nodes - i -1; j++) {
            printf("\t node %ld icf is %ld\n", j+1+i, findheapnode(heaps, i, j+i+1, 0)->cost);
            printf("\t node %ld icp is %ld\n", j+1+i, findheapnode(heaps, i, j+i+1, 1)->cost);
        }
        printf("\tmapping\n");
        for (j = 0; j < nodes - i -1; j++) {
            printf("\t %ld should be = %ld\n", j+i+1, findheapnode(heaps, i, j+i+1, 0)->node);
            printf("\t %ld should be = %ld\n", j+i+1, findheapnode(heaps, i, j+i+1, 1)->node);
            printf("\t node %ld maps to %ld\n", findheapnode(heaps, i, j+i+1, 0)->node, heaps[i*2].mapping[j]);
            printf("\t node %ld maps to %ld\n", findheapnode(heaps, i, j+i+1, 1)->node, heaps[i*2+1].mapping[j]);
        }
    }
}
#endif


kernel_heapnode_t *findheapnode(kernel_heap_t *heaps, graph_index_t node1, graph_index_t node2, int forbOrperm) {
    kernel_heap_t heap;
    graph_index_t tmp;

    if (node1 > node2) {
        tmp = node1;
        node1 = node2;
        node2 = tmp;
    }
    heap = heaps[node1*2+forbOrperm];
    return &heap.heap[heap.mapping[node2-node1-1]];
}




void restoreProp(kernel_heap_t *heaps, graph_index_t node1, graph_index_t node2, int forbOrperm) {
    graph_index_t tmpidx, changed, p, childindex;
    kernel_heapnode_t tmp, l, r, max;
    kernel_heap_t heap;
    int bubbletype;

    if (node1 > node2) {
        tmpidx = node1;
        node1 = node2;
        node2 = tmpidx;
    }
    
    heap = heaps[node1*2+forbOrperm];
    changed = heap.mapping[node2-node1-1];
    if(heap.heap[changed].cost > heap.heap[(changed-1)/2].cost) {
        bubbletype = 0;
    }
    else {
        bubbletype = 1;
    }

    if (bubbletype == 0) {
        while( changed > 0 ) {
            p = (changed-1)/2;
            if(heap.heap[changed].cost > heap.heap[p].cost) {
                tmp = heap.heap[p];
                heap.heap[p] = heap.heap[changed];
                heap.heap[changed] = tmp;
                heap.mapping[heap.heap[p].node-node1-1] = p;
                heap.mapping[heap.heap[changed].node-node1-1] = changed;
            }
            else {
                break;
            }
            changed = p;
        }
    }

    /* value has been decreased and we should bubble down */
    if (bubbletype == 1 && heap.size > 1) {
        while(changed <= (heap.size-2)/2) {
            l = heap.heap[2*changed+1];
            /* check that the right child exists */
            if(2*changed+2 == heap.size){
                max = l;
                childindex = 2*changed+1;
            }
            else {
                r = heap.heap[2*changed+2];
                max = l.cost > r.cost ? l:r;
                childindex = l.cost > r.cost ? 2*changed+1:2*changed+2;
            }
            if(heap.heap[changed].cost < max.cost) {
                heap.heap[childindex] = heap.heap[changed];
                heap.heap[changed] = max;
                heap.mapping[max.node-node1-1] = changed;
                heap.mapping[heap.heap[childindex].node-node1-1] = childindex;
            }
            else {
                break;
            }
            changed = childindex;
        }
    }
}


void getMax(kernel_heap_t *heaps, graph_value_t *maxIcp, graph_value_t *maxIcf, kernel_heap_t *bestIcp, kernel_heap_t *bestIcf, graph_index_t nodes) {
    graph_index_t i;
    *maxIcf = 0;
    *maxIcp = 0;
    for (i = 0; i < nodes*2-2; i += 2) {
        if (heaps[i].node < 0) {
            continue;
        }
        if (heaps[i].heap[0].cost >= *maxIcf) {
            *maxIcf = heaps[i].heap[0].cost;
            *bestIcf = heaps[i];
        }
        if (heaps[i+1].heap[0].cost >= *maxIcp) {
            *maxIcp = heaps[i+1].heap[0].cost;
            *bestIcp = heaps[i+1];
        }
    }
}
