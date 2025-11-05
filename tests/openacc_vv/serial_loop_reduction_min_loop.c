#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(c[0:10*n], minimum[0:10])
#pragma acc serial
#pragma acc loop gang private(temp)
#pragma acc loop reduction(min:temp)
#pragma acc loop
