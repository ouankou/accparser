#pragma acc data copyin(a[0:n])
#pragma acc serial loop reduction(&&:result)
