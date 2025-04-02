## Results and their brief interpretation. 

System -- Ubuntu on WSL2

RAM - DDR4 (speed?) 8GB (on WSL2)

Processor - AMD Ryzen 9 Mobile, 8 cores, 16 Threads

Three results have been shown: without optimization, with level-3 optimization, and with level-4 optimization.

Notice the drastic change from no optimization to level-3 optimization. At O3, the compiler aggressively unrolls the loops. The outcome is seen in the serial implementations and the parallel(first iterate parallelized) cache access optimized run. 

The other parallel runs, however, do not benefit much from optimization. 

Command: g++ matrix_multiply.cpp -o matrix_multiply -fopenmp -O3
