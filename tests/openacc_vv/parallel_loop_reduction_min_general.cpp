#pragma acc data copyin(a[0:n], b[0:n]) copy(min)
#pragma acc parallel loop reduction(min:min)
#pragma acc data copyin(a[0:10*n], b[0:10*n])
#pragma acc parallel loop reduction(min:minimums)
