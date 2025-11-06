#pragma acc data copyin(a[0:10*n], b[0:10*n]) copyout(c[0:10*n]) copy(totals[0:10])
#pragma acc serial
#pragma acc loop gang private(temp)
#pragma acc loop worker reduction(*:temp)
#pragma acc loop worker
