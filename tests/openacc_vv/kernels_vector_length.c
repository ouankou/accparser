#pragma acc data copyin(a[0:n]) copyout(b[0:n])
#pragma acc kernels vector_length(16)
#pragma acc loop vector
