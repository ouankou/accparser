#pragma acc data copyin(a[0:10*n], b[0:10*n]) create(c[0:10*n]) copyout(d[0:10*n])
#pragma acc kernels loop gang private(avg)
#pragma acc loop worker reduction(+:avg)
#pragma acc loop worker
