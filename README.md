# ALPBench-RISCV-PORT
RISCV PORT of All Level P arallelisim Benchmarks (ALPBench) 


Changes to the makefile 

1--> point to your required cross compiler.

for example in my case

CC = riscv64-unknown-linux-gnu-gcc
LD = riscv64-unknown-linux-gnu-gcc 

--> #set the required number of threads. multiple threads will be used and each thread will compute a portion of the app
USE_THREADS = -DTHRD -DNUM_THREADS=16  

-->
read the Readme file in each directory and follow the instructions to run the relevant benchmark 
