#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(min[0:10])
#pragma acc serial
#pragma acc loop gang private(temp)
#pragma acc loop vector reduction(min:temp)
