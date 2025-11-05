#pragma acc data copyin(a[0:10*n], b[0:10*n]) create(c[0:10*n]) copyout(d[0:10*n])
#pragma acc parallel loop gang private(avg)
#pragma acc loop worker reduction(+:avg)
#pragma acc loop worker
#pragma acc data copyin(a[0:25*n], b[0:25*n]) copyout(c[0:25*n], d[0:25*n])
#pragma acc parallel loop gang private(avgs)
#pragma acc loop worker reduction(+:avgs)
#pragma acc loop worker
