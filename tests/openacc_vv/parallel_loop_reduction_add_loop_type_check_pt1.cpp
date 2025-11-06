#pragma acc data copyin(a[0:10*n],b[0:10*n], c[0:10*n]) copyout(d[0:10*n])
#pragma acc parallel loop gang private(total)
#pragma acc loop worker reduction(+:total)
#pragma acc loop worker
