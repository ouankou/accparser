#pragma acc data copyin(a[0:n], b[0:n]) copy(total)
#pragma acc parallel loop reduction(+:total)
#pragma acc data copyin(a[0:10*n], b[0:10*n])
#pragma acc parallel loop reduction(+:c[0:10])
