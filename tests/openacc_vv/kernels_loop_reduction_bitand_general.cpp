#pragma acc data copyin(a[0:n])
#pragma acc kernels loop reduction(&:b)
