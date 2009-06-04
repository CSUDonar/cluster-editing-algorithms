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
#ifndef FMEM_H
#define FMEM_H

#include <stdlib.h>

/* Allocate a memory block */
void *fmem_alloc( size_t size );

/* Allocate an array */
void *fmem_alloc_arr( size_t size, size_t count );


/* Also allow null pointers
 */
void fmem_free( void *ptr );

/* Free memory block and set pointer to NULL.
 * Usage: fmem_free_h( &ptr );
 */
void fmem_free_h( void **handle );

#endif
