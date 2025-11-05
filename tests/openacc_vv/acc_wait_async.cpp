#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n], g[0:n]) create(c[0:n], f[0:n], h[0:n]) copyout(i[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc wait(2)
