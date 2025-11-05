#pragma acc routine (fmin) seq
#pragma acc data copyin(a[0:n], b[0:n]) copy(min)
#pragma acc kernels loop reduction(min:min)
