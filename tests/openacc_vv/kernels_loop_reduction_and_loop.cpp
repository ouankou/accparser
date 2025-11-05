#pragma acc data copy(a[0:10*n])
#pragma acc kernels loop gang private(temp)
#pragma acc loop worker reduction(&&:temp)
#pragma acc loop worker
