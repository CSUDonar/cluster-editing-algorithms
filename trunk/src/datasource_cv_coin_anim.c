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
#include <stdio.h>
#include <highgui.h>
#include <cv.h>

#include "graph.h"
#include "fmem.h"
#include "datasource.h"
#include "datasource_cv_coin_anim.h"
#include "cv_coin.h"

void *datasource_cv_coin_anim_create( char *args );
void datasource_cv_coin_anim_free( void *storage );
graph_t *datasource_cv_coin_anim_get( void *storage );
void datasource_cv_coin_anim_show( void *storage, graph_t *graph, graph_index_t *cliqueid );

const datasource_t datasource_cv_coin_anim = {
    datasource_cv_coin_anim_create,
    datasource_cv_coin_anim_free,
    datasource_cv_coin_anim_get,
    datasource_cv_coin_anim_show
};

typedef struct datasource_cv_coin_anim_storage_t {
    IplImage        *img;
    char            *filename;
    cv_coin_t       *cvc;
    int             param1;
    int             param2;
    IplImage        *smallimg;
} datasource_cv_coin_anim_storage_t;


void *datasource_cv_coin_anim_create( char *args ) {
    datasource_cv_coin_anim_storage_t *s = fmem_alloc( sizeof( datasource_cv_coin_anim_storage_t ) );

    s->filename = args;
    s->img = cvLoadImage( s->filename, CV_LOAD_IMAGE_COLOR );

    s->smallimg = cvCreateImage( cvSize( 900, 600 ), 8, 3 );

    if( !s->img ) {
        return NULL;
    }
    s->cvc = cv_coin_create( s->img );

    s->param1 = 100;
    s->param2 = 30;

    cvNamedWindow( "mainWin", CV_WINDOW_AUTOSIZE );
    cvMoveWindow( "mainWin", 10, 10 );

    cvCreateTrackbar( "param1", "mainWin", &s->param1, 255, NULL );
    cvCreateTrackbar( "param2", "mainWin", &s->param2, 255, NULL );

    return (void*)s;
}

void datasource_cv_coin_anim_free( void *storage ) {
    datasource_cv_coin_anim_storage_t *s = (datasource_cv_coin_anim_storage_t *)storage;

    cv_coin_free( s->cvc );
    fmem_free( storage );
}

graph_t *datasource_cv_coin_anim_get( void *storage ) {
    datasource_cv_coin_anim_storage_t *s = (datasource_cv_coin_anim_storage_t *)storage;
    graph_t         *g = NULL;
    
    if( cvWaitKey(10) >= 0) {
        return NULL;
    }

    s->img = cvLoadImage( s->filename, CV_LOAD_IMAGE_COLOR );
    g = cv_coin_get( s->cvc, s->img, s->param1, s->param2 );
    cvReleaseImage( &s->img );

    return  g;
}

void datasource_cv_coin_anim_show( void *storage, graph_t *graph, graph_index_t *cliqueid ) {
    datasource_cv_coin_anim_storage_t *s = (datasource_cv_coin_anim_storage_t *)storage;
    IplImage *dispimg = cv_coin_postprocess( s->cvc, graph, cliqueid );

    cvResize( dispimg, s->smallimg, CV_INTER_LINEAR );

    cvShowImage( "mainWin", s->smallimg );
}
