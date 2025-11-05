#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc kernels
#pragma acc loop worker
#pragma acc loop worker
