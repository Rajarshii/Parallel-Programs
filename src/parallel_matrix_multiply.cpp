#include <iostream>
#include <omp.h>
#include <chrono>
#include <vector>
#include <assert.h>
#include <random>

#define DIM_N 2048

// Shared memories 
int a[DIM_N][DIM_N];
int b[DIM_N][DIM_N];
int c[DIM_N][DIM_N];
int c_exp[DIM_N][DIM_N];

// Lambda for computing the execution time /// Scott Meyers implementation
auto timeFuncInvocation = 
    [](auto&& func, auto&&... params) {
        // get time before function invocation
        const auto& start = std::chrono::high_resolution_clock::now();
        // function invocation using perfect forwarding
        std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
        // get time after function invocation
        const auto& stop = std::chrono::high_resolution_clock::now();
        return stop - start;
     };


// Matrix initialization
void init_matrix () {
    int i, j;
    // initialize
    //#pragma omp parallel for schedule(dynamic,24) collapse(2) private(i,j) shared(a,b,c)
    for(i=0; i<DIM_N; ++i) {
        for(j=0; j<DIM_N; j++) {
            a[i][j] = std::rand() % 100;
            b[i][j] = std::rand() % 100;
            //c[i][j] = 0;
        }
    }
}

// Reset c
void reset_output_matrix () {
    int i, j;
    // initialize
    //#pragma omp parallel for schedule(dynamic,24) collapse(2) private(i,j) shared(a,b,c)
    for(i=0; i<DIM_N; ++i) {
        for(j=0; j<DIM_N; j++) {
            c[i][j] = 0;
        }
    }
}

// Check the result of multiplication
void check() {
    int i,j;
    //#pragma omp parallel for schedule(runtime) collapse(2) private(i,j)
    for(i=0; i<DIM_N; ++i) {
        for(j=0; j<DIM_N; ++j) {
            assert(c_exp[i][j] == c[i][j]);
        }
    }
}

// Serial naive implementation
void matrix_serial () {
    int i, j, k;
    for(i=0; i < DIM_N; ++i) {  
        for(j=0; j < DIM_N; ++j) {
            c_exp[i][j] = 0;
            for(k=0; k < DIM_N; ++k) {
                c_exp[i][j] += a[i][k]*b[k][j]; 
            }
        }
    }
}

// Parallel static implementation
void matrix_parallel_static (int& n) {
    int i, j, k;
    #pragma omp parallel for schedule(static,n) collapse(2) private(i,j,k) shared(a,b,c)
    for(i=0; i < DIM_N; ++i) {  
        for(j=0; j < DIM_N; ++j) {
            c[i][j] = 0;
            //#pragma omp simd reduction(+:c[i][j])
            for(k=0; k < DIM_N; ++k) {
                c[i][j] += a[i][k]*b[k][j]; 
            }
        }
    }
}

// Parallel static implementation - with SIMD for the 1D vector multiplication
void matrix_parallel_static_simd (int& n) {
    int i, j, k;
    #pragma omp parallel for schedule(static,n) collapse(2) private(i,j,k) shared(a,b,c)
    for(i=0; i < DIM_N; ++i) {  
        for(j=0; j < DIM_N; ++j) {
            int psum = 0;
            #pragma omp simd reduction(+:psum)
            for(k=0; k < DIM_N; ++k) {
                psum += a[i][k]*b[k][j]; 
            }
            c[i][j] = psum;
        }
    }
}

// Parallel static Dynamic
void matrix_parallel_dynamic (int& n) {
    int i, j, k;
    #pragma omp parallel for schedule(dynamic,n) collapse(2) private(i,j,k) shared(a,b,c)
    for(i=0; i < DIM_N; ++i) {  
        for(j=0; j < DIM_N; ++j) {
            c[i][j] = 0;
            for(k=0; k < DIM_N; ++k) {
                c[i][j] += a[i][k]*b[k][j]; 
            }
        }
    }
}

// Serial with cache optimization
void matrix_cache_row () {
    int i, j, k;
    for(i=0; i < DIM_N; ++i) {   // Row wise multiplication
        for(k=0; k < DIM_N; ++k) {
            for(j=0; j < DIM_N; ++j) {
                c[i][j] += a[i][k]*b[k][j]; 
            }
        }
    }
}

// Serial with bad locality - column wise access of a
void matrix_cache_col () {
    int i, j, k;
    for(j=0; j < DIM_N; ++j) {   // Column wise multiplication
        for(k=0; k < DIM_N; ++k) {
            for(i=0; i < DIM_N; ++i) {
                c[i][j] += a[i][k]*b[k][j]; 
            }
        }
    }
}

// parallel with cache optimization
void matrix_parallel_cache (int& n) {
    int i, j, k;
    #pragma omp parallel for schedule(dynamic,n) private(i,j,k) shared(a,b,c)
    for(i=0; i < DIM_N; ++i) {   // Row wise multiplication
        for(k=0; k < DIM_N; ++k) {
            for(j=0; j < DIM_N; ++j) {
                c[i][j] += a[i][k]*b[k][j]; 
            }
        }
    }
}

int main () {

    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    omp_set_nested(1);
    int max_threads = omp_get_max_threads();
    std::cout << "max_threads:" << max_threads << '\n';

    init_matrix();

    //Serial naive approach
    auto ms_int = duration_cast<milliseconds>(timeFuncInvocation(matrix_serial));
    // Parallel - cache - column first
    reset_output_matrix();
    auto ms_int5 = duration_cast<milliseconds>(timeFuncInvocation(matrix_cache_row));
    check();
    /*reset_output_matrix();
    auto ms_int6 = duration_cast<milliseconds>(timeFuncInvocation(matrix_cache_col));
    check();*/
    std::cout << "serial:" << ms_int.count() 
    << ",cache_row:" << ms_int5.count() 
    //<< ",cache_col:" << ms_int6.count()
    << '\n' ; 

    for(int n=20; n <= 500; n=n+20) {
    // Parallel - with static scheduling
    auto ms_int1 = duration_cast<milliseconds>(timeFuncInvocation(matrix_parallel_static,n));
    check();
    // Parallel - with dynamic scheduling
    auto ms_int2 = duration_cast<milliseconds>(timeFuncInvocation(matrix_parallel_dynamic,n));
    check();
    // Parallel - with static scheduling and simd
    auto ms_int3 = duration_cast<milliseconds>(timeFuncInvocation(matrix_parallel_static_simd,n));
    check();
    // Parallel - cache - row first
    reset_output_matrix();
    auto ms_int4 = duration_cast<milliseconds>(timeFuncInvocation(matrix_parallel_cache,n));
    check();
    // Print the results
    std::cout << "block_size:" << n
    << ",static:" << ms_int1.count() << ",dynamic:" 
    << ms_int2.count() << ",simd:" << ms_int3.count()
    << ",cache_parallel:" << ms_int4.count() 
    << '\n';

    }

    return 0;
}