#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(c[0:10*n], maximum[0:10])
#pragma acc kernels loop gang private(temp)
#pragma acc loop worker reduction(max:temp)
#pragma acc loop worker
