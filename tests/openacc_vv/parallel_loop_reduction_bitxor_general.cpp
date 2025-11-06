#pragma acc data copyin(a[0:n])
#pragma acc parallel loop reduction(^:b)
#pragma acc data copyin(a[0:10*n])
#pragma acc parallel loop reduction(^:b)
