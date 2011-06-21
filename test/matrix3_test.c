#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "seatest.h"
#include "matrix_3.h"

/* Accuracy of float comparisons */
#define delta 0.00000000000001 
void print_matrix(matrix_3 a){
    printf("%f,%f,%f\n%f,%f,%f\n%f,%f,%f\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]);
}

void test_matrices_equal(matrix_3 a,matrix_3 b){
    int i;
    for(i=0;i<9;i++){
        assert_float_equal(a[i],b[i],delta);
    }
}

/* Test a set of properties of determinants to check that
   the calculation is right */
void test_determinant(){
    printf("Testing determinant calculations.\n");
    /* Identity matrix det = 1 */
    matrix_3 I = {1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0};
    assert_float_equal(1.0,m3_det(I),delta);
    
    /* 0 row so det = 0 */
    matrix_3 j = {0.0,0.0,0.0,2.0,4.0,5.0,6.0,7.0,8.0};
    assert_float_equal(0.0,m3_det(j),delta);
    
    /* 0 column so det = 0 */
    matrix_3 m = {1.0,0.0,3.0,2.0,0.0,5.0,6.0,0.0,8.0};
    assert_float_equal(0.0,m3_det(m),delta); 
    
    /* det(transpose(A)) = det(A) */
    matrix_3 k = {2.8,4.6,5.7,8.9,2.3,44.4,6.3,8.0,4.7};
    matrix_3 l = {2.8,8.9,6.3,4.6,2.3,8.0,5.7,44.4,4.7};
    assert_float_equal(m3_det(k),m3_det(l),delta);
    
    /* swap columns then det(A') = -det(A) */
    matrix_3 k1 = {2.8,5.7,4.6,8.9,44.4,2.3,6.3,4.7,8.0};
    assert_float_equal(-1*m3_det(k),m3_det(k1),delta);
    
    /* triangular matrix, det = product of diagonal entries */
    matrix_3 n = {0.0,0.0,54.47,0.0,34.87,44.47,96.30,88.70,84.77};
    assert_float_equal(-1*54.47 * 34.87 * 96.30,m3_det(n),delta);
    
}

void test_inverse(){
    printf("Testing matrix inversion\n");
    matrix_3 I = {1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0};
    matrix_3 j;
    m3_invert(I,j);
    test_matrices_equal(I,j);
     
    matrix_3 k = {23.6,54.76,42.2,34.1,64.7,44.7,42.3,63.2,64.9};
    matrix_3 l;
    m3_invert(k,j);
    m3_invert(j,l);
    test_matrices_equal(k,l);
    
    matrix_3 n;
    m3_multiply(k,j,n);
    test_matrices_equal(n,I);
    
    /* Non invertible matrix */
    matrix_3 p = {0.0,0.0,0.0,2.0,0.0,4.0,0.0,0.0,0.0};
    matrix_3 q;
    m3_invert(p,q); 
    int i;
    for(i=0;i<9;i++){
      assert_true(isnan(q[i])); 
    }
}

void test_multiply(){
    printf("Testing matrix multiplication\n");
    matrix_3 m = {1.0,5.0,2.0,-1.0,0.0,1.0,3.0,2.0,4.0};
    matrix_3 n = {6.0,1.0,3.0,-1.0,1.0,2.0,4.0,1.0,3.0};
    matrix_3 p;
    matrix_3 q = {9.0,8.0,19.0,-2.0,0.0,0.0,32.0,9.0,25.0};
    matrix_3 r = {14.0,36.0,25.0,4.0,-1.0,7.0,12.0,26.0,21.0};
    
    m3_multiply(m,n,p);
    test_matrices_equal(p,q);
    m3_multiply(n,m,p);
    test_matrices_equal(p,r);
    
    /* Identity */
    matrix_3 I = {1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0};
    m3_multiply(r,I,p);
    test_matrices_equal(p,r);
    m3_multiply(I,r,p);
    test_matrices_equal(p,r);
    
}

void test_matrix3( void )
{
	test_fixture_start();
	run_test(test_determinant); 
	run_test(test_inverse); 
    run_test(test_multiply);  
	test_fixture_end();       
}




