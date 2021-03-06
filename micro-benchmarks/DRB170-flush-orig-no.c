/*
!!!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!
!!! Copyright (c) 2017-20, Lawrence Livermore National Security, LLC
!!! and DataRaceBench project contributors. See the DataRaceBench/COPYRIGHT file for details.
!!!
!!! SPDX-License-Identifier: (BSD-3-Clause)
!!!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!
 */
/**
 * LU.C: This file is part of kernel of the NAS Parallel Benchmarks 3.0 LU suit.
 * Tsan and Intel Inspector can not correctly analyze the flush, and report a false postive.
*/

#include <stdio.h>
#include <stdbool.h>
#define ISIZ 12
#define	OMEGA_DEFAULT	1.2

#if defined(_OPENMP)
/* for thread synchronization */
static bool flag[ISIZ/2*2+1];
#endif /* _OPENMP */

int main(int argh, char* argv[])
{
  int i,j,k,m,ist,iend,jst,jend,nx,ny,nz,iglob,xi,jglob,eta,zeta;
  double omega;
  double v[ISIZ][ISIZ/2*2+1][ISIZ/2*2+1][5];
  omega = OMEGA_DEFAULT;
  nx = ISIZ;
  ny = ISIZ;
  nz = ISIZ;
  ist = 1;
  iend = nx-2;
  jst =1;
  jend = ny -2;
  
/* inintal value */
  for (i = 0; i < nx; i++) {
    iglob = i;
    xi = ( (double)(iglob) ) / ( nx - 1 );
    for (j = 0; j < ny; j++) {
      jglob = j;
      eta = ( (double)(jglob) ) / ( ny - 1 );
      for (k = 0; k < nz; k++) {
        zeta = ( (double)(k) ) / ( nz - 1 );
        for (m = 0; m < 5; m++) {
            v[i][j][k][m] = xi + eta +  zeta;
        }
      }
    }
  }
    #pragma omp parallel private(k)
    {
    for(k = 1; k <= nz - 2; k++){

    #pragma omp for nowait schedule(static)
    for (i = ist; i <= iend; i++) {
      for (j = jst; j <= jend; j++) {
        for (m = 0; m < 5; m++) {
      v[i][j][k][m] = v[i][j][k][m]
        - omega * (  v[i][j][k-1][0]
                 + v[i][j][k-1][1]
                 + v[i][j][k-1][2]
                 + v[i][j][k-1][3]
                 + v[i][j][k-1][4]  );
        }
      }
    }
#pragma omp for nowait schedule(static)
      for (i = ist; i <= iend; i++) {

    #if defined(_OPENMP)      
      if (i != ist) {
        while (flag[i-1] == 0) {
          #pragma omp flush(flag)
            ;
        }
      }
      if (i != iend) {
        while (flag[i] == 1) {
          #pragma omp flush(flag)
            ;
        }
      }
    #endif /* _OPENMP */
        
    for (j = jst; j<= jend; j++) {
      for (m = 0; m < 5; m++) {
        v[i][j][k][m] = v[i][j][k][m]
            - omega * ( v[i][j-1][k][0]
                      + v[i][j-1][k][1]
                      + v[i-1][j][k][1]
                      + v[i][j-1][k][2]
                      + v[i-1][j][k][3]
                      + v[i-1][j][k][4] );
      }
    v[i][j][k][1] = v[i][j][k][1]
          - v[i][j][k][0];
    }

    #if defined(_OPENMP)    
      if (i != ist) flag[i-1] = 0;
      if (i != iend) flag[i] = 1;
      #pragma omp flush(flag)    
    #endif /* _OPENMP */    
    }
  }
 }
  return 0;
}
