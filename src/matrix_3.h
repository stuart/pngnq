#ifndef _INC_MATRIX3_H
#define _INC_MATRIX3_H
/* Functions to deal with 3x3 matrices of doubles */ 

typedef double matrix_3[9];

/* Determinant of a 3x3 matrix. */
double m3_det(matrix_3 m);

/* Inversion of 3x3 matrix. 
   m - matrix to invert.
   Result stored in i. 
*/
void m3_invert(matrix_3 m, matrix_3 i);

/* Multiplication of 2 3x3 matrices a and b.
   Result in c
*/
void m3_multiply(matrix_3 a, matrix_3 b, matrix_3 c);

#endif