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
#ifndef GRAPHFILE_H
#define GRAPHFILE_H

#include <stdio.h>
#include "graph.h"

graph_t *graphfile_readfobj( FILE *fp );
graph_t *graphfile_readfile( const char *filename );

int graphfile_writefobj( const graph_t *graph, FILE *fp );
int graphfile_writefile( const graph_t *graph, const char *filename );

#endif
