#pragma acc data copyin(a[0:10*n]) copy(b[0:10])
#pragma acc kernels loop private(c)
#pragma acc loop vector reduction(&:c)
