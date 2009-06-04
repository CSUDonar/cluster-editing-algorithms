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
#include "cv_coin.h"
#include "visual.h"

cv_coin_t *cv_coin_create( IplImage *firstimage ) {
    cv_coin_t *cvc = fmem_alloc( sizeof( cv_coin_t ) );

    cvc->storage = cvCreateMemStorage(0);

    cvc->imgdisp = cvCreateImage(cvSize( firstimage->width, firstimage->height ), 8, 3);
    cvc->imgcoltest = cvCreateImage(cvSize( firstimage->width, firstimage->height ), 8, 3);
    cvc->gray = cvCreateImage(cvSize( firstimage->width, firstimage->height ), 8, 1);

    cvInitFont( &cvc->font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 1, 8 );

    return cvc;
}

void cv_coin_free( cv_coin_t *cvc ) {
    cvReleaseImage( &cvc->gray );
    cvReleaseImage( &cvc->imgcoltest );
    cvReleaseImage( &cvc->imgdisp );
    cvReleaseMemStorage( &cvc->storage );
    fmem_free( cvc );
}

graph_t *cv_coin_get( cv_coin_t *cvc, IplImage *img, int param1, int param2 ) {
    graph_t         *g = NULL;
    graph_index_t   i, j;
    graph_value_t   val;
    float           *pi, *pj;
    float           diff;
    float           vx, vy, vl;
    CvScalar        ci, cj;

    cvCopy( img, cvc->imgdisp, NULL );

    cvCvtColor( img, cvc->gray, CV_BGR2GRAY );
    cvSmooth( cvc->gray, cvc->gray, CV_GAUSSIAN, 7, 7, 0, 0 );
//    cvThreshold( cvc->gray, cvc->gray, 0.3, 1.0, CV_THRESH_BINARY );

    cvCvtColor( img, cvc->imgcoltest, CV_BGR2YCrCb );
    cvSmooth( cvc->imgcoltest, cvc->imgcoltest, CV_GAUSSIAN, 15, 15, 0, 0 );

    cvc->circles = cvHoughCircles(cvc->gray, cvc->storage, CV_HOUGH_GRADIENT, 1, cvc->gray->height/20, param1, param2, 50, 120);

    g = graph_create( cvc->circles->total );

    for( i=0; i<cvc->circles->total; i++ ) {
        pi = (float*)cvGetSeqElem( cvc->circles, i );
        ci = cvGet2D( cvc->imgcoltest, cvRound( pi[1] ), cvRound( pi[0] ) );

        for( j=i+1; j<cvc->circles->total; j++ ) {
            pj = (float*)cvGetSeqElem( cvc->circles, j );
            cj = cvGet2D( cvc->imgcoltest, cvRound( pj[1] ), cvRound( pj[0] ) );

            diff = pi[2]-pj[2];
            if( diff < 0.0 ) {
                diff = -diff;
            }

            val = (7 - diff) * 3;

            diff = 0;
/*            diff += (ci.val[0]-cj.val[0])*(ci.val[0]-cj.val[0])/10.0;*/
            diff += (ci.val[1]-cj.val[1])*(ci.val[1]-cj.val[1]);
            diff += (ci.val[2]-cj.val[2])*(ci.val[2]-cj.val[2]);
            diff = sqrt( diff );

            if( diff < 6.0 ) {
                diff = 6.0;
            }

            val += (6.0 - diff)*1.5;

            if( val >= 0 ) {
                val++;
            }

            graph_setValue( g, i, j, val );
            if( val > 0 ) {

                vx = pi[0] - pj[0];
                vy = pi[1] - pj[1];
                vl = sqrt( vx*vx + vy*vy );
                vx /= vl;
                vy /= vl;

                cvLine( cvc->imgdisp,
                        cvPoint( cvRound( pi[0] - vx*pi[2] ), cvRound( pi[1] - vy*pi[2] ) ),
                        cvPoint( cvRound( pj[0] + vx*pj[2] ), cvRound( pj[1] + vy*pj[2] ) ),
                        CV_RGB( 0, 255, 0 ),
                        3,
                        8, 0);
            }
        }
    }

    return  g;
}

IplImage *cv_coin_postprocess( cv_coin_t *cvc, graph_t *graph, graph_index_t *cliqueid ) {
    graph_index_t i, j, cnt;
    graph_index_t ncliques=1;
    float *p;

    float r[256]; /* Assumes less than 256 cliques */
    unsigned char n[256];
    graph_index_t tmp;

    char string[256];


    for( i=0; i<cvc->circles->total; i++ ) {
        if( cliqueid[i] >= ncliques ) {
            ncliques = cliqueid[i]+1;
        }
    }

    for( i = 0; i < ncliques; i++ ) {
        n[i] = 0;
        r[i] = 0.0;
    }
    for( i = 0; i < cvc->circles->total; i++ ) {
        p = (float*)cvGetSeqElem( cvc->circles, i );
        n[cliqueid[i]]++;
        r[cliqueid[i]]+=p[2];
    }
    for( i = 0; i < ncliques; i++ ) {
        r[i] /= n[i];
    }


    for( i=0; i<cvc->circles->total; i++ ) {
        p = (float*)cvGetSeqElem( cvc->circles, i );
        cvCircle( cvc->imgdisp, cvPoint( cvRound(p[0]),cvRound(p[1]) ),cvRound(p[2]),
                CV_RGB(
                    ((1+cliqueid[i])&2)?(255):(0),
                    ((1+cliqueid[i])&4)?(255):(0),
                    ((1+cliqueid[i])&1)?(255):(0)
                    ), 3, 8, 0 );
    }

    for( i = 0; i < ncliques; i++ ) {
        cvCircle( cvc->imgdisp, cvPoint( 20, 20 + 30*i ), 5,
                CV_RGB(
                    ((1+i)&2)?(255):(0),
                    ((1+i)&4)?(255):(0),
                    ((1+i)&1)?(255):(0)
                    ), 3, 8, 0 );
        sprintf( string, "%d: %d", i, n[i] );
        cvPutText( cvc->imgdisp, string, cvPoint( 40, 30 + 30*i ), &cvc->font, CV_RGB( 255, 0, 0 ) );
    }

//    return cvc->gray;
    return cvc->imgdisp;
}
