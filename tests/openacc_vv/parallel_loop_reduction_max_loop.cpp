#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(c[0:10*n], maximum[0:10])
#pragma acc parallel loop gang private(temp)
#pragma acc loop worker reduction(max:temp)
#pragma acc loop worker
#pragma acc data copyin(a[0:25*n], b[0:25*n]) copy(c[0:25*n], maximum[0:25])
#pragma acc parallel loop gang private(temp)
#pragma acc loop worker reduction(max:temp)
#pragma acc loop worker
