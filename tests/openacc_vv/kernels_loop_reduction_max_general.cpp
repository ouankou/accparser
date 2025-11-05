#pragma acc data copyin(a[0:n], b[0:n]) copy(max)
#pragma acc kernels loop reduction(max:max)
