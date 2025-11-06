#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel loop reduction(+:total)
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel loop reduction(+:total)
