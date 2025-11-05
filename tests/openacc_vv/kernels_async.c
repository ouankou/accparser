#pragma acc enter data create(g[0:n], c[0:n], f[0:n])
#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n])
#pragma acc kernels async(1)
#pragma acc loop
#pragma acc kernels async(2)
#pragma acc loop
#pragma acc kernels wait(1, 2) async(3)
#pragma acc loop
#pragma acc wait(1, 2)
#pragma acc update host(c[0:n], f[0:n])
#pragma acc exit data copyout(g[0:n]) async(3)
#pragma acc wait(3)
#pragma acc exit data delete(c[0:n], f[0:n])
