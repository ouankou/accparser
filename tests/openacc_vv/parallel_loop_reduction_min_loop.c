#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(c[0:10*n], minimum[0:10])
#pragma acc parallel loop gang private(temp)
#pragma acc loop reduction(min:temp)
#pragma acc loop
#pragma acc data copyin(a[0:25*n], b[0:25*n]) copy(minimums[0:25], c[0:25*n])
#pragma acc parallel loop gang private(reduced)
#pragma acc loop reduction(min:reduced)
#pragma acc loop
