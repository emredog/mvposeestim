#include "passmsg.h"

#include "component.h"
#include "logging.h"


// message passing

// Source: mex_unix/shiftdt.cc

#define INF 1E20
#include <math.h>
#include <sys/types.h>
#include <float.h> // for DBL_MAX

/*
 * shiftdt.cc
 * Generalized distance transforms based on Felzenswalb and Huttenlocher.
 * This applies computes a min convolution of an arbitrary quadratic function ax^2 + bx
 * This outputs results on an shifted, subsampled grid (useful for passing messages between variables in different domains)
 */

static inline int square(int x) { return x*x; }

#if NO_DT

//function that calculates ax*dx^2 + bx*dx + ay*dy^2 + by*dy
static inline FPTYPE deformationScore(const FPTYPE &w1, const FPTYPE &w2, const FPTYPE &w3, const FPTYPE &w4,
                                      const int &c_col, const int &c_row, const int &p_col, const int &p_row,
                                      int offsetX, int offsetY)
{
    // Read in deformation coefficients, negating to define a cost
    FPTYPE ax = -w1;
    FPTYPE bx = -w2;
    FPTYPE ay = -w3;
    FPTYPE by = -w4;

    int dx = c_col - p_col - offsetX;
    int dy = c_row - p_row - offsetY;

    //return ax*(FPTYPE)dx + bx*(FPTYPE)square(dx) + ay*(FPTYPE)dy + by*(FPTYPE)square(dy);
    return bx*(FPTYPE)dx + ax*(FPTYPE)square(dx) + by*(FPTYPE)dy + ay*(FPTYPE)square(dy);

}
#endif

// dt1d(source,destination_val,destination_ptr,source_step,source_length,
//      a,b,dest_shift,dest_length,dest_step)
void dt1d(FPTYPE *src, FPTYPE *dst, int *ptr, int step, int len, FPTYPE a, FPTYPE b, int dshift, int dlen, int dstep)
{
    int   *v = new int[len];
    float *z = new float[len+1];
    int k = 0;
    int q = 0;
    v[0] = 0;
    z[0] = -INF;
    z[1] = +INF;

    for (q = 1; q <= len-1; q++)
    {
        float s = ((src[q*step] - src[v[k]*step]) - b*(q - v[k]) + a*(square(q) - square(v[k]))) / (2*a*(q-v[k]));
        while (s <= z[k])
        {
            k--;
            s  = ((src[q*step] - src[v[k]*step]) - b*(q - v[k]) + a*(square(q) - square(v[k]))) / (2*a*(q-v[k]));
        }
        k++;
        v[k]   = q;
        z[k]   = s;
        z[k+1] = +INF;
    }

    k = 0;
    q = dshift;

    for (int i=0; i <= dlen-1; i++)
    {
        while (z[k+1] < q)
            k++;
        dst[i*step] = a*square(q-v[k]) + b*(q-v[k]) + src[v[k]*step];
        ptr[i*step] = v[k];
        q += dstep;
    }

    delete [] v;
    delete [] z;
}

void shiftdt_C(myArray<FPTYPE> *child_score, FPTYPE child_w_1, FPTYPE child_w_2, FPTYPE child_w_3, FPTYPE child_w_4, FPTYPE child_startx, FPTYPE child_starty, int Nx, int Ny, FPTYPE child_step, /*outputs*/ myArray<FPTYPE> **score0, myArray<int> **Ix0, myArray<int> **Iy0)
{
    // Read in deformation coefficients, negating to define a cost
    // Read in offsets for output grid, fixing MATLAB 0-1 indexing
    FPTYPE *vals = child_score->getPtr();
    int sizx  = child_score->getCols();
    int sizy  = child_score->getRows();
    FPTYPE ax = -child_w_1;
    FPTYPE bx = -child_w_2;
    FPTYPE ay = -child_w_3;
    FPTYPE by = -child_w_4;
    int offx  = (int)child_startx - 1;
    int offy  = (int)child_starty - 1;
    int lenx  = Nx;
    int leny  = Ny;
    int step  = (int)child_step;

    myArray<FPTYPE> *mxM = new myArray<FPTYPE>(leny, lenx);
    myArray<int> *mxIy = new myArray<int>(leny, lenx);
    myArray<int> *mxIx = new myArray<int>(leny, lenx);
    FPTYPE  *M = mxM->getPtr();
    int *Iy = mxIy->getPtr();
    int *Ix = mxIx->getPtr();

    FPTYPE *tmpM =  (FPTYPE *) calloc(leny*sizx, sizeof(FPTYPE));
    int *tmpIy = (int *) calloc(leny*sizx, sizeof(int));

    //printf("(%d,%d),(%d,%d),(%d,%d)\n",offx,offy,lenx,leny,sizx,sizy);

    // dt1d(source,destination_val,destination_ptr,source_step,source_length,
    //      a,b,dest_shift,dest_length,dest_step)
    for (int x = 0; x < sizx; x++)
        dt1d(vals+x*sizy, tmpM+x*leny, tmpIy+x*leny, 1, sizy, ay, by, offy, leny, step);

    for (int y = 0; y < leny; y++)
        dt1d(tmpM+y, M+y, Ix+y, leny, sizx, ax, bx, offx, lenx, step);

    // get argmins and adjust for matlab indexing from 1
    for (int x = 0; x < lenx; x++)
    {
        for (int y = 0; y < leny; y++)
        {
            int p = x*leny+y;
            Iy[p] = tmpIy[Ix[p]*leny+y]+1;
            Ix[p] = Ix[p]+1;
        }
    }

    free(tmpM);
    free(tmpIy);

    *score0 = mxM;
    *Ix0 = mxIx;
    *Iy0 = mxIy;

    return;
}

#if NO_DT
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Second version of passmsg, WITHOUT the distance transform
// as input arg, it doesn't take all scores in the matrix, but only the
// the ones that we picked as "promising"

void passmsg_noDT(int childId, int parentId, const Component *parts, myArray<FPTYPE> *promisingPositions[],
                 /*outputs*/ myArray<FPTYPE> **score, myArray<int> **_IchildPos)
{
    //we don't need all the scores anymore, just the ones that we chose as "promising"
    //all child positions (contains 4 rows: [col row fi score]^T )
    myArray<FPTYPE> *child_positions = promisingPositions[childId-1];
    //all parent positions (contains 4 rows: [col row fi score]^T )
    myArray<FPTYPE> *parent_positions = promisingPositions[parentId-1];

    int nbOfPromisingPts = child_positions->getCols(); // each column is an instance of promising pt. the amount should be same for parent and child.

    // these ones will still store the indices | one entry for each row, single column only
    myArray<int> **IchildPos = &(_IchildPos[childId-1]); // will contain, index of best scoring child position for parent of child
    // e.g. IchildPos[26]->get(39, 0, 0) is the best child index for part 26, given the index (39) of its parent
    *IchildPos = new myArray<int>(nbOfPromisingPts, 1, 1);
    *score = new myArray<FPTYPE>(nbOfPromisingPts, 1, 1);

    // fetch weights for child
    FPTYPE *child_w = parts->w->getPtr(0, 0, childId-1);
    int wrows = parts->w->getRows(); // number or rows in w array

    // we dont need to iterate over filterIds, positions for this part is already mixed filter types
    // and we dont do Distance Transform. Instead, we iterate over all promising positions of child, for all promising positions of parent

    int c_col = -1, c_row = -1, c_fi = -1, p_col = -1, p_row = -1, p_fi = -1;
    FPTYPE c_sc = -1.0;
    FPTYPE totalScore = 0.0;

    FPTYPE *child_startx = parts->startx->getPtr(0, 0, childId-1);
    FPTYPE *child_starty = parts->starty->getPtr(0, 0, childId-1);

    //debug
    //    std::cout << "Parent id = " << parentId << " Child id = " << childId << std::endl;
    //    std::cout << "===================================================================\n";
    //    double maxB, maxChildScore;
    //debug

    for (int pi=0; pi < nbOfPromisingPts; pi++) //iterate over all parent promising positions
    {
        FPTYPE maxScore = -DBL_MAX; // ---> best match between this parent and all of it child locations
        int bestMatchingChildIndex = -1;

        p_col = static_cast<int>(parent_positions->get(COL, pi)); //col of this prom. pt
        p_row = static_cast<int>(parent_positions->get(ROW, pi)); //row of this prom. pt
        p_fi  = static_cast<int>(parent_positions->get(FI,  pi)); //fi  of this prom. pt

        for (int ci=0; ci < nbOfPromisingPts; ci++) //iterate over all child promising positions
        {
            c_col = static_cast<int>(child_positions->get(COL, ci)); //col of this prom. pt
            c_row = static_cast<int>(child_positions->get(ROW, ci)); //row of this prom. pt
            c_fi  = static_cast<int>(child_positions->get(FI, ci));  //filter id of this prom. pt
            c_sc  = child_positions->get(SCORE, ci);                 //score of this prom. pt
            int offx  = (int)child_startx[c_fi-1] - 1;
            int offy  = (int)child_starty[c_fi-1] - 1;

            // fetch weights (learned & stored in model)
            FPTYPE w1 = child_w[(c_fi-1)*wrows];        //1st weight of this filter c_fi (weight is a 4 element row vector, (w^titj)_ij in the paper)
            FPTYPE w2 = child_w[(c_fi-1)*wrows + 1];    //2nd weight of this filter c_fi
            FPTYPE w3 = child_w[(c_fi-1)*wrows + 2];    //3rd weight of this filter c_fi
            FPTYPE w4 = child_w[(c_fi-1)*wrows + 3];    //4th weight of this filter c_fi

            // calculate deformation score given weights and positions of parent and child
            FPTYPE defScore = deformationScore(w1, w2, w3, w4, c_col, c_row, p_col, p_row, offx, offy);

            //debug
            //if (defScore > 0)
            //{
            //  int dx = abs(p_col - c_col);
            //  int dy = abs(p_row - c_row);
            //  std::cout << "dx = " << dx << "\tdy = " << dy << "\tEuclidianDistance = " << sqrt(double(dx*dx) + double(dy*dy))
            //          << "\tDefScore = " << defScore << std::endl;
            //}
            //debug

            //add BIAS (b^titj)_ij in the paper
            FPTYPE b = parts->b->get(p_fi-1, c_fi-1, childId-1);

            totalScore = defScore + c_sc + b; // w*DISTANCEs + CHILDSCORE + Bij

            if (totalScore > maxScore)
            {
                maxScore = totalScore;
                bestMatchingChildIndex = ci;
                //debug
                //maxChildScore = c_sc;
                //maxB = b;
                //debug
            }
        }

        if (bestMatchingChildIndex < 0) // it's pretty unlikely..
        {
            std::cerr << "\n\n[ERROR] passmsg_noDT - No valid parent position candidate for child: " << childId << " in position (" << c_col << ", " << c_row
                      << ")\n\tMax score of parent candidates: " << maxScore << std::endl;
            continue;
        }

        (*score)->set(maxScore, pi, 0, 0);
        (*IchildPos)->set(bestMatchingChildIndex, pi, 0, 0);
        //debug
        //        std::cout << "Best match p_col: " << p_col << "\tp_row: " << p_row << "\tp_fi: " << p_fi
        //                  << "\tIS c_col: " << child_positions->get(COL, bestMatchingChildIndex)
        //                  << "\tc_row: " << child_positions->get(ROW, bestMatchingChildIndex)
        //                  << "\tc_fi: " << child_positions->get(FI, bestMatchingChildIndex) << "\tWITH score: " << maxScore
        //                  << "\tChildscore = " << maxChildScore << "\tb = " << maxB << std::endl;
        //debug
    }
}
#endif
//--------------------------------------------------------------------



//--------------------------------------------------------------------

// C version of passmsg() function defined in detect_fast.m
// Given a 2D array of filter scores 'child',
// (1) Apply distance transform
// (2) Shift by anchor position of part wrt parent
// (3) Downsample if necessary
void passmsg_C(int childId, int parentId, const Component *parts, myArray<FPTYPE> **parts_score[], /*outputs*/ myArray<FPTYPE> **score, myArray<int> **_Ix, myArray<int> **_Iy, myArray<int> **_Ik)
{
    myArray<FPTYPE> **child_score = parts_score[childId-1];
    myArray<FPTYPE> **parent_score = parts_score[parentId-1];
    myArray<int> **Ix = &(_Ix[childId-1]);
    myArray<int> **Iy = &(_Iy[childId-1]);
    myArray<int> **Ik = &(_Ik[childId-1]);


    //? INF = 1e10;
    int K = parts->filterid->getCols();
    int Ny = parent_score[0]->getRows(); // number of rows
    int Nx = parent_score[0]->getCols(); // number of cols

    //? [Ix0,Iy0,score0] = deal(zeros([Ny Nx K]));

#if LOGGING_ON
    snprintf(buffer, sizeof(buffer), "passmsg K=%d Ny=%d Nx=%d", K, Ny, Nx);
    writeLog(buffer);
#endif

    FPTYPE *child_w = parts->w->getPtr(0, 0, childId-1);
    int wrows = parts->w->getRows(); // number or rows in w array

    FPTYPE *child_startx = parts->startx->getPtr(0, 0, childId-1);
    FPTYPE *child_starty = parts->starty->getPtr(0, 0, childId-1);
    FPTYPE child_step = parts->step->get(childId-1);

    myArray<FPTYPE> *score0[K];
    myArray<int> *Ix0[K];
    myArray<int> *Iy0[K];

    for( int k = 1; k <= K; k++) // here k is a filter id, NOT a part id
    {
#if 0
        snprintf(buffer, sizeof(buffer), "passmsg k=%d child.w(1,k)=%f child.w(2,k)=%f child.w(3,k)=%f child.w(4,k)=%f child.startx(k)=%f child.starty(k)=%f child.step=%f", k, child_w[(1-1)+(k-1)*wrows], child_w[(2-1)+(k-1)*wrows], child_w[(3-1)+(k-1)*wrows], child_w[(4-1)+(k-1)*wrows], child_startx[k-1], child_starty[k-1], child_step);
        writeLog(buffer);
        writeLog("passmsg child.score(:,:,k)=");
        writeLog(parts_score[childId-1][k-1]/*child_score[k-1]*/);
#endif

        shiftdt_C(child_score[k-1], child_w[(1-1)+(k-1)*wrows], child_w[(2-1)+(k-1)*wrows], child_w[(3-1)+(k-1)*wrows], child_w[(4-1)+(k-1)*wrows], child_startx[k-1], child_starty[k-1], Nx, Ny, child_step, /*outputs*/ &score0[k-1], &Ix0[k-1], &Iy0[k-1]);

#if 0
        writeLog("passmsg score0=");
        writeLog(score0[k-1]);
        writeLog("passmsg Ix0=");
        writeLog(Ix0[k-1]);
        writeLog("passmsg Iy0=");
        writeLog(Iy0[k-1]);
#endif
    }

    // At each parent location, for each parent mixture 1:L, compute best child mixture 1:K
    int L = parts->filterid->getCols();

#if LOGGING_ON
    snprintf(buffer, sizeof(buffer), "passmsg L=%d", L);
    writeLog(buffer);
#endif

    *score = new myArray<FPTYPE>(Ny, Nx, L); // 3D array, Ny x Nx x L
    *Ix = new myArray<int>(Ny, Nx, L);
    *Iy = new myArray<int>(Ny, Nx, L);
    *Ik = new myArray<int>(Ny, Nx, L);

    for( int l = 1; l <= L; l++) // l = 1:6
    {
        for( int r = 1; r <= Ny; r++)
        {
            for( int c = 1; c <= Nx; c++)
            {
                FPTYPE *_score = (*score)->getPtr(r-1, c-1, l-1);
                *_score = -DBL_MAX;

                // max of each cell on z dimension
                for( int z = 1; z <= K; z++) // z = 1:6
                {
                    FPTYPE b = parts->b->get(l-1, z-1, childId-1);
                    FPTYPE _score0 = score0[z-1]->get(r-1, c-1);
                    FPTYPE val = _score0 + b;

                    if( val > *_score )
                    {
                        // new max at position (r, c, z)
                        *_score = val;
                        (*Ix)->set(Ix0[z-1]->get(r-1, c-1), r-1, c-1, l-1);
                        (*Iy)->set(Iy0[z-1]->get(r-1, c-1), r-1, c-1, l-1);
                        (*Ik)->set(z, r-1, c-1, l-1);
                    }
                }
            }
        }
    }

    // release memory
    for(int k = 0; k < K; k++)
    {
        delete score0[k];
        delete Ix0[k];
        delete Iy0[k];
    }

#if LOGGING_ON
    writeLog("passmsg score=");
    writeLog(*score);
    writeLog("passmsg Ix=");
    writeLog(*Ix);
    writeLog("passmsg Iy=");
    writeLog(*Iy);
    writeLog("passmsg Ik=");
    writeLog(*Ik);
#endif
}
