/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

void trans(int M, int N, int A[N][M], int B[M][N]);

void trans1(int M, int N, int A[N][M], int B[M][N])
{
    int i,j, ib, jb, d, temp;
    for(i=0; i<32; i+=8){
        for(j=0; j<32; j+=8)
            for(ib=i; ib<i+8; ib++){
                {
                    for(jb=j; jb<j+8; jb++)
                    {
                        if(i == j && ib == jb)
                            {
                                d = ib;
                                temp = A[d][d];
                            }
                        else
                            B[jb][ib] = A[ib][jb];
                    }
                    if(i==j)
                    {
                        B[d][d] = temp;
                    }
                }
            }
    }
}

void trans2(int M, int N, int A[N][M], int B[M][N])
{
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int i;
    for(M=0; M<64; M+=8)
    {
        for(N=0; N<64; N+=8)
        {
			for(i = 0; i < 4; i++)
			{
				temp1 = A[M + i][N + 0];
				temp2 = A[M + i][N + 1];
				temp3 = A[M + i][N + 2];
				temp4 = A[M + i][N + 3];
				temp5 = A[M + i][N + 4];
				temp6 = A[M + i][N + 5];
				temp7 = A[M + i][N + 6];
				temp8 = A[M + i][N + 7];
				
                B[N + 0][M + i] = temp1;
				B[N + 0][M + 4 + i] = temp5;
				B[N + 1][M + i] = temp2;
				B[N + 1][M + 4 + i] = temp6;
				B[N + 2][M + i] = temp3;
				B[N + 2][M + 4 + i] = temp7;
				B[N + 3][M + i] = temp4;
				B[N + 3][M + 4 + i] = temp8;
			}
    
            for(i=0; i<4; i++)
            {
                temp1 = A[M + 4][N + i];
                temp2 = A[M + 5][N + i];
                temp3 = A[M + 6][N + i];
                temp4 = A[M + 7][N + i];

                temp5 = B[N + i][M + 4];
                temp6 = B[N + i][M + 5];
                temp7 = B[N + i][M + 6];
                temp8 = B[N + i][M + 7];

                B[N + i][M + 4]=temp1;
                B[N + i][M + 5]=temp2;
                B[N + i][M + 6]=temp3;
                B[N + i][M + 7]=temp4;

                B[N + 4 + i][M]=temp5;
                B[N + 4 + i][M + 1]=temp6;
                B[N + 4 + i][M + 2]=temp7;
                B[N + 4 + i][M + 3]=temp8;

            }

			for(i = 4; i < 8; i+=2)
            {   
            temp5 = A[M + i + 1][N + 4];
            temp6 = A[M + i + 1][N + 5];
            temp7 = A[M + i + 1][N + 6];
            temp8 = A[M + i + 1][N + 7];
            temp1 = A[M + i][N + 4];
            temp2 = A[M + i][N + 5];
            temp3 = A[M + i][N + 6];
            temp4 = A[M + i][N + 7];
            B[N + 4][M + i] = temp1;
            B[N + 5][M + i] = temp2;
            B[N + 6][M + i] = temp3;
            B[N + 7][M + i] = temp4;
            B[N + 4][M + i + 1] = temp5;
            B[N + 5][M + i + 1] = temp6;
            B[N + 6][M + i + 1] = temp7;
            B[N + 7][M + i + 1] = temp8;
            }
        }
    }
}


void trans3(int M, int N, int A[N][M], int B[M][N])
{    
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int i;
    for(N=0; N<64; N+=8)
    {
        for(M=0; M<56; M+=8)
        {
			for(i = 0; i < 4; i++)
			{
				temp1 = A[N + i][M + 0];
				temp2 = A[N + i][M + 1];
				temp3 = A[N + i][M + 2];
				temp4 = A[N + i][M + 3];
				temp5 = A[N + i][M + 4];
				temp6 = A[N + i][M + 5];
				temp7 = A[N + i][M + 6];
				temp8 = A[N + i][M + 7];
				
                B[M + 0][N + i] = temp1;
				B[M + 0][N + 4 + i] = temp5;
				B[M + 1][N + i] = temp2;
				B[M + 1][N + 4 + i] = temp6;
				B[M + 2][N + i] = temp3;
				B[M + 2][N + 4 + i] = temp7;
				B[M + 3][N + i] = temp4;
				B[M + 3][N + 4 + i] = temp8;
				
			}
    
            for(i=0; i<4; i++)
            {
                temp1 = A[N + 4][M + i];
                temp2 = A[N + 5][M + i];
                temp3 = A[N + 6][M + i];
                temp4 = A[N + 7][M + i];

                temp5 = B[M + i][N + 4];
                temp6 = B[M + i][N + 5];
                temp7 = B[M + i][N + 6];
                temp8 = B[M + i][N + 7];

                B[M + i][N + 4]=temp1;
                B[M + i][N + 5]=temp2;
                B[M + i][N + 6]=temp3;
                B[M + i][N + 7]=temp4;

                B[M + 4 + i][N]=temp5;
                B[M + 4 + i][N + 1]=temp6;
                B[M + 4 + i][N + 2]=temp7;
                B[M + 4 + i][N + 3]=temp8;

            }

			for(i = 4; i < 8; i+=2)
            {   
            temp5 = A[N + i+1][M + 4];
            temp6 = A[N + i+1][M + 5];
            temp7 = A[N + i+1][M + 6];
            temp8 = A[N + i+1][M + 7];
            temp1 = A[N + i][M + 4];
            temp2 = A[N + i][M + 5];
            temp3 = A[N + i][M + 6];
            temp4 = A[N + i][M + 7];
            B[M + 4][N+i] = temp1;
            B[M+ 5][N+i] = temp2;
            B[M+ 6][N+i] = temp3;
            B[M+ 7][N+i] = temp4;
            B[M + 4][N+i+1] = temp5;
            B[M+ 5][N+i+1] = temp6;
            B[M+ 6][N+i+1] = temp7;
            B[M+ 7][N+i+1] = temp8;
            }
        }

        for(i = 0; i < 8; i++)
        {
            temp1 = A[N + i][M + 0];
            temp2 = A[N + i][M + 1];
            temp3 = A[N + i][M + 2];
            temp4 = A[N + i][M + 3];
            temp5 = A[N + i][M + 4];
            
            B[M + 0][N + i] = temp1;
            B[M + 1][N + i] = temp2;
            B[M + 2][N + i] = temp3;
            B[M + 3][N + i] = temp4;
            B[M + 4][N + i] = temp5;
        }
    }
    for(M=0; M<60; M+=4)
    {
        for(i = 0; i < 3; i++)
        {
            temp1 = A[N + i][M + 0];
            temp2 = A[N + i][M + 1];
            temp3 = A[N + i][M + 2];
            temp4 = A[N + i][M + 3];
            
            B[M + 0][N + i] = temp1;
            B[M + 1][N + i] = temp2;
            B[M + 2][N + i] = temp3;
            B[M + 3][N + i] = temp4;
        }
    }
    temp1 = A[64][60];
    temp2 = A[65][60];
    temp3 = A[66][60];
    
    B[60][64] = temp1;
    B[60][65] = temp2;
    B[60][66] = temp3;
}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M==32)
        trans1(M,N,A,B);
    else if(M==64)
        trans2(M,N,A,B);
    else if(M==61)
        trans3(M,N,A,B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

