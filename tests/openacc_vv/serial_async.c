#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n], c[0:n], f[0:n], g[0:n])
#pragma acc serial async(1)
#pragma acc loop
#pragma acc serial async(2)
#pragma acc loop
#pragma acc serial wait(1, 2) async(3)
#pragma acc loop
#pragma acc update host(c[0:n]) async(1)
#pragma acc update host(f[0:n]) async(2)
#pragma acc update host(g[0:n]) async(3)
#pragma acc wait(1)
#pragma acc wait(2)
#pragma acc wait(3)
