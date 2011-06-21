/* Yes, I am including the .c files here */
#include "matrix3_test.c"
#include "colorspace_test.c"


void all_tests( void )
{
    test_matrix3();
    test_colorspace();   
}

int main( int argc, char** argv )
{       
    return run_tests(all_tests);
}
