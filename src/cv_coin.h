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
#ifndef CV_COIN_T
#define CV_COIN_T

typedef struct cv_coin_t {
    IplImage        *img;
    IplImage        *gray;
    IplImage        *imgcoltest;
    IplImage        *imgdisp;
    CvMemStorage    *storage;
    CvSeq           *circles;
    CvFont          font;
} cv_coin_t;

cv_coin_t *cv_coin_create( IplImage *tmplimage );
void cv_coin_free( cv_coin_t *cvc );
graph_t *cv_coin_get( cv_coin_t *cvc, IplImage *img, int param1, int param2 );
IplImage *cv_coin_postprocess( cv_coin_t *cvc, graph_t *graph, graph_index_t *cliqueid );

#endif
