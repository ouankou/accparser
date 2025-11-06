#pragma acc data copy(a[0:n], b[0:n], c[0:n], d2[0:n*n])
#pragma acc kernels loop tile(*, *) reduction(+:temp)
#pragma acc data copyin(a[0:n], b[0:n], c[0:n]) copyout(d3[0:n*n*n])
#pragma acc kernels loop tile(n/10, n, n*2)
