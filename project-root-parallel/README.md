## Parallel implementation

```bash
make               # compile everything
make run-seq       # run sequentially (1 thread)
make run-parallel  # run with 4 threads
make run-parallel PARALLEL_THREADS=8  # run with 8 threads
make clean         # clean up
