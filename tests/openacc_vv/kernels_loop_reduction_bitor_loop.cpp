#pragma acc data copyin(a[0:10*n]) copy(b[0:10*n], c[0:10])
#pragma acc kernels loop gang private(temp)
#pragma acc loop worker reduction(|:temp)
#pragma acc loop worker
