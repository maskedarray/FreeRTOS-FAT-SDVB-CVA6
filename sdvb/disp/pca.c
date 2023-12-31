/***********************  Contents  ****************************************
 * Principal Components Analysis: C, 638 lines. ****************************
 * Sample input data set (final 36 lines). *********************************
 ***************************************************************************
 */

/*********************************/
/* Principal Components Analysis */
/*********************************/

/*********************************************************************/
/* Principal Components Analysis or the Karhunen-Loeve expansion is a
   classical method for dimensionality reduction or exploratory data
   analysis.  One reference among many is: F. Murtagh and A. Heck,
   Multivariate Data Analysis, Kluwer Academic, Dordrecht, 1987.

Author:
F. Murtagh
Phone:        + 49 89 32006298 (work)
+ 49 89 965307 (home)
Earn/Bitnet:  fionn@dgaeso51,  fim@dgaipp1s,  murtagh@stsci
Span:         esomc1::fionn
Internet:     murtagh@scivax.stsci.edu

F. Murtagh, Munich, 6 June 1989                                   */   
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_ramdisk.h"
#include "ff_sys.h"
#include "pca.h"

#define SIGN(a, b) ( (b) < 0 ? -fabs(a) : fabs(a) )

int my_scanf(FF_FILE *stream, int *result) {
    int ch;
    int value = 0;
    int digitEncountered = 0;

    // Skip non-digit characters
    while ((ch = ff_fgetc(stream)) != EOF && !isdigit(ch));

    // Read digits until a non-digit character is encountered
    while (ch != EOF && isdigit(ch)) {
        digitEncountered = 1;
        value = value * 10 + (ch - '0');
        ch = ff_fgetc(stream);
    }

    // If at least one digit was encountered, update the result
    if (digitEncountered) {
        *result = value;
        return 1;  // Successfully read an integer
    } else {
        return 0;  // No digit was encountered
    }
}

void benchmark_pca()
{
  FF_FILE *stream;
  int  n, m,  i, j, k, k2;
  float **data, **matrix(), **symmat, **symmat2, *vector(), *evals, *interm;
  void free_matrix(), free_vector(), corcol(), covcol(), scpcol();
  void tred2(), tqli();
  float in_value;
  char option, *strncpy();

  /*********************************************************************
    Get from command line:
    input data file name, #rows, #cols, option.

    Open input file: fopen opens the file whose name is stored in the
    pointer argv[argc-1]; if unsuccessful, error message is printed to
    stderr.
   *********************************************************************/

 

  n = 4435;              /* # rows */
  m = 37;              /* # columns */
  option = 'R';

  FF_PRINTF("No. of rows: %d, no. of columns: %d.\n",n,m);
  // printf("Input file: %s.\n",argv[1]);
  stream = ff_fopen("/ram/pca/sat.trn","r");
  if (stream == NULL)
  {
    // fprintf(stderr, "Program : cannot open file \n");
    FF_PRINTF("Program : cannot open file \n");
    while(1){}
    // fprintf(stderr, "Exiting to system.");
    // exit(1);
    /* Note: in versions of DOS prior to 3.0, argv[0] contains the
       string "C". */
  }

  /* Now read in data. */

  data = matrix(n, m);  /* Storage allocation for input data */

  for (i = 1; i <= n; i++)
  {
    for (j = 1; j <= m; j++)
    {
      my_scanf(stream, &in_value);
      // fscanf(stream, "%f", &in_value);
      data[i][j] = in_value;
      // FF_PRINTF("at row %d column %d is %d\n",i,j,(int)data[i][j]);
    }
  }



  /* Check on (part of) input data.
     for (i = 1; i <= 18; i++) {for (j = 1; j <= 8; j++)  {
     printf("%7.1f", data[i][j]);  }  printf("\n");  }
     */



  symmat = matrix(m, m);  /* Allocation of correlation (etc.) matrix */

  /* Look at analysis option; branch in accordance with this. */

  switch(option)
  {
    case 'R':
    case 'r':
      FF_PRINTF("Analysis of correlations chosen.\n");
      corcol(data, n, m, symmat);

      /* Output correlation matrix.
         for (i = 1; i <= m; i++) {
         for (j = 1; j <= 8; j++)  {
         printf("%7.4f", symmat[i][j]);  }
         printf("\n");  }
         */
      break;
    case 'V':
    case 'v':
      FF_PRINTF("Analysis of variances-covariances chosen.\n");
      covcol(data, n, m, symmat);

      /* Output variance-covariance matrix.
         for (i = 1; i <= m; i++) {
         for (j = 1; j <= 8; j++)  {
         printf("%7.1f", symmat[i][j]);  }
         printf("\n");  }
         */
      break;
    case 'S':
    case 's':
      FF_PRINTF("Analysis of sums-of-squares-cross-products");
      FF_PRINTF(" matrix chosen.\n");
      scpcol(data, n, m, symmat);

      /* Output SSCP matrix.
         for (i = 1; i <= m; i++) {
         for (j = 1; j <= 8; j++)  {
         printf("%7.1f", symmat[i][j]);  }
         printf("\n");  }
         */
      break;
    default:
      FF_PRINTF("Option: %s\n",option);
      FF_PRINTF("For option, please type R, V, or S\n");
      FF_PRINTF("(upper or lower case).\n");
      FF_PRINTF("Exiting to system.\n");
      while(1){}
      // exit(1);
      break;
  }

  /*********************************************************************
    Eigen-reduction
   **********************************************************************/

  /* Allocate storage for dummy and new vectors. */
  evals = vector(m);     /* Storage alloc. for vector of eigenvalues */
  FF_PRINTF("the vector storage size is %d\n",m);
  interm = vector(m);    /* Storage alloc. for 'intermediate' vector */
  symmat2 = matrix(m, m);  /* Duplicate of correlation (etc.) matrix */
  for (i = 1; i <= m; i++) {
    for (j = 1; j <= m; j++) {
      symmat2[i][j] = symmat[i][j]; /* Needed below for col. projections */
    }
  }
  tred2(symmat, m, evals, interm);  /* Triangular decomposition */
  FF_PRINTF("eval value at 0 is %d\n",(int)evals[0]);
  FF_PRINTF("eval value at 1 is %d\n",(int)evals[1]);
  FF_PRINTF("eval value at 2 is %d\n",(int)evals[2]);

  FF_PRINTF("m/height is %d \n",(int)m);

  FF_PRINTF("m/height is %d \n",m);
  FF_PRINTF("m/height is %d \n",n);
  FF_PRINTF("m/height is %d \n",n);


  tqli(evals, interm, m, symmat);   /* Reduction of sym. trid. matrix */
  /* evals now contains the eigenvalues,
     columns of symmat now contain the associated eigenvectors. */

  FF_PRINTF("\nEigenvalues:\n");
  for (j = m; j >= 1; j--) {
    FF_PRINTF("%d\n", (int)evals[j]); }
  FF_PRINTF("\n(Eigenvalues should be strictly positive; limited\n");
  FF_PRINTF("precision machine arithmetic may affect this.\n");
  FF_PRINTF("Eigenvalues are often expressed as cumulative\n");
  FF_PRINTF("percentages, representing the 'percentage variance\n");
  FF_PRINTF("explained' by the associated axis or principal component.)\n");

  FF_PRINTF("\nEigenvectors:\n");
  FF_PRINTF("(First three; their definition in terms of original vbes.)\n");
  for (j = 1; j <= m; j++) {
    for (i = 1; i <= 3; i++)  {
      FF_PRINTF("%d", (int)symmat[j][m-i+1]);  }
    FF_PRINTF("\n");  }




  /* Form projections of row-points on first three prin. components. */
  /* Store in 'data', overwriting original data. */
  for (i = 1; i <= n; i++) {
    for (j = 1; j <= m; j++) {
      interm[j] = data[i][j];
    //  FF_PRINTF("Iteration i=%d j=%d data = %d\n\n", i,j,(int)data[i][j]);

    }   
    for (k = 1; k <= 3; k++) {
      data[i][k] = 0.0;
      for (k2 = 1; k2 <= m; k2++) {
        data[i][k] += interm[k2] * symmat[k2][m-k+1]; 
    //  FF_PRINTF("Iteration i= %d, j = %d k=%d, k2=%d : data = %d\n",i,j,k, k2,(int)data[i][k]);

      }
     FF_PRINTF("\n");

    }
  }











  FF_PRINTF("\nProjections of row-points on first 3 prin. comps.:\n");
  for (i = 1; i <= n; i++) {
    for (j = 1; j <= 3; j++)  {
      FF_PRINTF("%d", (int)data[i][j]);  }
    FF_PRINTF("\n");  }
return;

  /* Form projections of col.-points on first three prin. components. */
  /* Store in 'symmat2', overwriting what was stored in this. */
  for (j = 1; j <= m; j++) {
    for (k = 1; k <= m; k++) {
      interm[k] = symmat2[j][k]; }  /*symmat2[j][k] will be overwritten*/
    for (i = 1; i <= 3; i++) {
      symmat2[j][i] = 0.0;
      for (k2 = 1; k2 <= m; k2++) {
        symmat2[j][i] += interm[k2] * symmat[k2][m-i+1]; }
      if (evals[m-i+1] > 0.0005)   /* Guard against zero eigenvalue */
        symmat2[j][i] /= sqrt(evals[m-i+1]);   /* Rescale */
      else
        symmat2[j][i] = 0.0;    /* Standard kludge */
    }
  }

  FF_PRINTF("\nProjections of column-points on first 3 prin. comps.:\n");
  for (j = 1; j <= m; j++) {
    for (k = 1; k <= 3; k++)  {
      FF_PRINTF("%d", (int)symmat2[j][k]);  }
    FF_PRINTF("\n");  }

  free_matrix(data, n, m);
  free_matrix(symmat, m, m);
  free_matrix(symmat2, m, m);
  free_vector(evals, m);
  free_vector(interm, m);

}

/**  Correlation matrix: creation  ***********************************/

void corcol(data, n, m, symmat)
  float **data, **symmat;
  int n, m;
  /* Create m * m correlation matrix from given n * m data matrix. */
{
  float eps = 0.005;
  float x, *mean, *stddev, *vector();
  int i, j, j1, j2;

  /* Allocate storage for mean and std. dev. vectors */

  mean = vector(m);
  stddev = vector(m);

  /* Determine mean of column vectors of input data matrix */

  for (j = 1; j <= m; j++)
  {
    mean[j] = 0.0;
    for (i = 1; i <= n; i++)
    {
      mean[j] += data[i][j];
    }
    mean[j] /= (float)n;
  }

  FF_PRINTF("\nMeans of column vectors:\n");
  for (j = 1; j <= m; j++)  {
    FF_PRINTF("%d",(int)mean[j]);  }   FF_PRINTF("\n");

  /* Determine standard deviations of column vectors of data matrix. */

  for (j = 1; j <= m; j++)
  {
    stddev[j] = 0.0;
    for (i = 1; i <= n; i++)
    {
      stddev[j] += (   ( data[i][j] - mean[j] ) *
          ( data[i][j] - mean[j] )  );
    }
    stddev[j] /= (float)n;
    stddev[j] = sqrt(stddev[j]);
    /* The following in an inelegant but usual way to handle
       near-zero std. dev. values, which below would cause a zero-
       divide. */
    if (stddev[j] <= eps) stddev[j] = 1.0;
  }

  FF_PRINTF("\nStandard deviations of columns:\n");
  for (j = 1; j <= m; j++) { FF_PRINTF("%d", (int)stddev[j]); }
  FF_PRINTF("\n");

  /* Center and reduce the column vectors. */

  for (i = 1; i <= n; i++)
  {
    for (j = 1; j <= m; j++)
    {
      data[i][j] -= mean[j];
      x = sqrt((float)n);
      x *= stddev[j];
      data[i][j] /= x;
      // FF_PRINTF("value is %d\n",(int)data[i][j]);
    }
  }

  /* Calculate the m * m correlation matrix. */
  for (j1 = 1; j1 <= m-1; j1++)
  {
    symmat[j1][j1] = 1.0;
    for (j2 = j1+1; j2 <= m; j2++)
    {
      symmat[j1][j2] = 0.0;
      for (i = 1; i <= n; i++)
      {
        symmat[j1][j2] += ( data[i][j1] * data[i][j2]);
          // FF_PRINTF("multiplying values [%d][%d] * [%d][%d]\n",i,j1,i,j2);
          // FF_PRINTF("Multiplying %d and %d\n",(int)data[i][j1],(int)data[i][j2]);

          // FF_PRINTF("Value at %d %d = %d\n",j1,j2,(int)symmat[j1][j2]);
      }
      // FF_PRINTF("**SPLIT**\n");
      // FF_PRINTF("swapping [%d] [%d] = [%d] [%d]\n",j2,j1, j1,j2);

      symmat[j2][j1] = symmat[j1][j2];
    }
  }
  symmat[m][m] = 1.0;


  return;

}

/**  Variance-covariance matrix: creation  *****************************/

void covcol(data, n, m, symmat)
  float **data, **symmat;
  int n, m;
  /* Create m * m covariance matrix from given n * m data matrix. */
{
  float *mean, *vector();
  int i, j, j1, j2;

  /* Allocate storage for mean vector */

  mean = vector(m);

  /* Determine mean of column vectors of input data matrix */

  for (j = 1; j <= m; j++)
  {
    mean[j] = 0.0;
    for (i = 1; i <= n; i++)
    {
      mean[j] += data[i][j];
    }
    mean[j] /= (float)n;
  }

  FF_PRINTF("\nMeans of column vectors:\n");
  for (j = 1; j <= m; j++)  {
    FF_PRINTF("%d",(int)mean[j]);  }   FF_PRINTF("\n");

  /* Center the column vectors. */

  for (i = 1; i <= n; i++)
  {
    for (j = 1; j <= m; j++)
    {
      data[i][j] -= mean[j];
    }
  }

  /* Calculate the m * m covariance matrix. */
  for (j1 = 1; j1 <= m; j1++)
  {
    for (j2 = j1; j2 <= m; j2++)
    {
      symmat[j1][j2] = 0.0;
      for (i = 1; i <= n; i++)
      {
        symmat[j1][j2] += data[i][j1] * data[i][j2];
      }
      symmat[j2][j1] = symmat[j1][j2];
    }
  }

  return;

}

/**  Sums-of-squares-and-cross-products matrix: creation  **************/

void scpcol(data, n, m, symmat)
  float **data, **symmat;
  int n, m;
  /* Create m * m sums-of-cross-products matrix from n * m data matrix. */
{
  int i, j1, j2;

  /* Calculate the m * m sums-of-squares-and-cross-products matrix. */

  for (j1 = 1; j1 <= m; j1++)
  {
    for (j2 = j1; j2 <= m; j2++)
    {
      symmat[j1][j2] = 0.0;
      for (i = 1; i <= n; i++)
      {
        symmat[j1][j2] += data[i][j1] * data[i][j2];
      }
      symmat[j2][j1] = symmat[j1][j2];
    }
  }

  return;

}

/**  Error handler  **************************************************/

void erhand(err_msg)
  char err_msg[];
  /* Error handler */
{
  while(1){}
  // fprintf(stderr,"Run-time error:\n");
  // fprintf(stderr,"%s\n", err_msg);
  // fprintf(stderr,"Exiting to system.\n");
  // exit(1);
}

/**  Allocation of vector storage  ***********************************/

float *vector(n)
  int n;
  /* Allocates a float vector with range [1..n]. */
{

  float *v;

  v = (float *) pvPortMalloc ((unsigned) n*sizeof(float));
  if (!v) erhand("Allocation failure in vector().");
  return v-1;

}

/**  Allocation of float matrix storage  *****************************/

float **matrix(n,m)
  int n, m;
  /* Allocate a float matrix with range [1..n][1..m]. */
{
  int i;
  float **mat;

  /* Allocate pointers to rows. */
  mat = (float **) pvPortMalloc((unsigned) (n)*sizeof(float*));
  if (!mat) erhand("Allocation failure 1 in matrix().");
  mat -= 1;

  /* Allocate rows and set pointers to them. */
  for (i = 1; i <= n; i++)
  {
    mat[i] = (float *) pvPortMalloc((unsigned) (m)*sizeof(float));
    if (!mat[i]) erhand("Allocation failure 2 in matrix().");
    mat[i] -= 1;
  }

  /* Return pointer to array of pointers to rows. */
  return mat;

}

/**  Deallocate vector storage  *********************************/

void free_vector(v,n)
  float *v;
  int n;
  /* Free a float vector allocated by vector(). */
{
  vPortFree((char*) (v+1));
}

/**  Deallocate float matrix storage  ***************************/

void free_matrix(mat,n,m)
  float **mat;
  int n, m;
  /* Free a float matrix allocated by matrix(). */
{
  int i;

  for (i = n; i >= 1; i--)
  {
    vPortFree ((char*) (mat[i]+1));
  }
  vPortFree ((char*) (mat+1));
}

/**  Reduce a real, symmetric matrix to a symmetric, tridiag. matrix. */

void tred2(a, n, d, e)
  float **a, *d, *e;
  /* float **a, d[], e[]; */
  int n;
  /* Householder reduction of matrix a to tridiagonal form.
Algorithm: Martin et al., Num. Math. 11, 181-195, 1968.
Ref: Smith et al., Matrix Eigensystem Routines -- EISPACK Guide
Springer-Verlag, 1976, pp. 489-494.
W H Press et al., Numerical Recipes in C, Cambridge U P,
1988, pp. 373-374.  */
{
  int l, k, j, i;
  float scale, hh, h, g, f;

  for (i = n; i >= 2; i--)
  {
    l = i - 1;
    h = scale = 0.0;
    if (l > 1)
    {
      for (k = 1; k <= l; k++)
        scale += fabs(a[i][k]);
      if (scale == 0.0)
        e[i] = a[i][l];
      else
      {
        for (k = 1; k <= l; k++)
        {
          a[i][k] /= scale;
          h += a[i][k] * a[i][k];
        }
        f = a[i][l];
        g = f>0 ? -sqrt(h) : sqrt(h);
        e[i] = scale * g;
        h -= f * g;
        a[i][l] = f - g;
        f = 0.0;
        for (j = 1; j <= l; j++)
        {
          a[j][i] = a[i][j]/h;
          g = 0.0;
          for (k = 1; k <= j; k++)
            g += a[j][k] * a[i][k];
          for (k = j+1; k <= l; k++)
            g += a[k][j] * a[i][k];
          e[j] = g / h;
          f += e[j] * a[i][j];
        }
        hh = f / (h + h);
        for (j = 1; j <= l; j++)
        {
          f = a[i][j];
          e[j] = g = e[j] - hh * f;
          for (k = 1; k <= j; k++)
            a[j][k] -= (f * e[k] + g * a[i][k]);
        }
      }
    }
    else
      e[i] = a[i][l];
    d[i] = h;
  }
  d[1] = 0.0;
  e[1] = 0.0;
  for (i = 1; i <= n; i++)
  {
    l = i - 1;
    if (d[i])
    {
      for (j = 1; j <= l; j++)
      {
        g = 0.0;
        for (k = 1; k <= l; k++)
          g += a[i][k] * a[k][j];
        for (k = 1; k <= l; k++)
          a[k][j] -= g * a[k][i];
      }
    }
    d[i] = a[i][i];
    a[i][i] = 1.0;
    for (j = 1; j <= l; j++)
      a[j][i] = a[i][j] = 0.0;
  }
}

/**  Tridiagonal QL algorithm -- Implicit  **********************/

void tqli(d, e, n, z)
  float d[], e[], **z;
  int n;
{
  int m, l, iter, i, k;
  float s, r, p, g, f, dd, c, b;
  void erhand();

  for (i = 2; i <= n; i++)
    e[i-1] = e[i];
  e[n] = 0.0;
  for (l = 1; l <= n; l++)
  {
    iter = 0;
    do
    {
      for (m = l; m <= n-1; m++)
      {
        dd = fabs(d[m]) + fabs(d[m+1]);
        if (fabs(e[m]) + dd == dd) break;
      }
      if (m != l)
      {
        if (iter++ == 30) erhand("No convergence in TLQI.");
        g = (d[l+1] - d[l]) / (2.0 * e[l]);
        r = sqrt((g * g) + 1.0);
        g = d[m] - d[l] + e[l] / (g + SIGN(r, g));
        s = c = 1.0;
        p = 0.0;
        for (i = m-1; i >= l; i--)
        {
          f = s * e[i];
          b = c * e[i];
          if (fabs(f) >= fabs(g))
          {
            c = g / f;
            r = sqrt((c * c) + 1.0);
            e[i+1] = f * r;
            c *= (s = 1.0/r);
          }
          else
          {
            s = f / g;
            r = sqrt((s * s) + 1.0);
            e[i+1] = g * r;
            s *= (c = 1.0/r);
          }
          g = d[i+1] - p;
          r = (d[i] - g) * s + 2.0 * c * b;
          p = s * r;
          d[i+1] = g + p;
          g = c * r - b;
          for (k = 1; k <= n; k++)
          {
            f = z[k][i+1];
            z[k][i+1] = s * z[k][i] + c * f;
            z[k][i] = c * z[k][i] - s * f;
          }
        }
        d[l] = d[l] - p;
        e[l] = g;
        e[m] = 0.0;
      }
    }  while (m != l);
  }
}


