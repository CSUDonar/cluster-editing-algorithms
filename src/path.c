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
/*
  n är antalet noder i pathen.
  path är en array med vilka noder som finns i pathen (i ordning i pathen)
  vals är en array av samma längd som path fylld med -1
  bestsplit är en array av samma längd som path, behöver inte vara satt till
  något speciellt
  graph är grafen kort och gott
 */
graph_value_t clusterPath(graph_index_t n, graph_index_t * path,
                          graph_value_t * vals, graph_index_t * bestsplit,
                          graph_t * graph) {
    
    graph_index_t i;
    graph_value_t cliqueCost;
    graph_value_t minCost;
    graph_index_t minSplitEdge;

    if (n == 0) {
        return 0;
    }

    if (vals[n - 1] != -1) {
        return vals[n - 1];
    }

    cliqueCost = 0;
    minCost = -1;
    minIndex = -1;
    for (i = n - 2; i >= 0; i--) {
        // cost to remove the edge in the path
        graph_value_t splitCost = graph_getValue(graph, path[i], path[i + 1]);
        // cost to cluster what's left of the removed edge in the best
        // possible way
        splitCost += clusterPath(i + 1, path, vals, bestsplit, graph);
        // cost the create a clique from what's right of the removed edge
        splitCost += (cliqueCost += addNode(i + 1, n, path, graph));
        if (minCost != -1) {
            if (splitCost < minCost) {
                minCost = splitCost;
                minSplitEdge = i;
            }
        }
        else {
            minCost = splitCost;
            minSplitEdge = i;
        }
    }
    // cost to create a clique of all nodes
    cliqueCost += addNode(0, n, path, graph);
    if (splitCost < minCost) {
        minCost = splitCost;
        minSplitEdge = -1;
    }
    
    bestsplit[n - 1] = minSplitEdge;
    vals[n - 1] = minCost;
    return vals[n - 1];
}

graph_value_t addNode(graph_index_t node, graph_index_t maxNode,
                      graph_index_t * path, graph_t * graph) {
    graph_value_t cost = 0;
    graph_index_t i;
    // already connected to node + 1 so add node + 2 and beyond
    for (i = node + 2; i < maxNode; i++) {
        // should be a non-edge, therefore -=
        cost -= graph_getValue(graph, path[node], path[i]);
    }
    return cost;
}
