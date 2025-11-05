#pragma acc data copyin(a[0:n])
#pragma acc parallel loop reduction(&&:result)
#pragma acc data copyin(a[0:5*n])
#pragma acc parallel loop reduction(&&:result)
