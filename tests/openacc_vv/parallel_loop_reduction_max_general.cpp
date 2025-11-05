#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel loop reduction(max:max)
#pragma acc data copyin(a[0:10*n], b[0:10*n])
#pragma acc parallel loop reduction(max:maximums)
