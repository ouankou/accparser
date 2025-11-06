#pragma acc data copyin(a[0:n]) copy(b[0:n])
#pragma acc kernels loop seq
