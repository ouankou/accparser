#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n], g[0:n], h[0:n]) create(c[0:n], f[0:n], i[0:n], j[0:n]) copyout(k[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc parallel async(3)
#pragma acc loop
#pragma acc parallel async(4)
#pragma acc loop
#pragma acc parallel async(1)
#pragma acc parallel async(2)
#pragma acc parallel async(3)
#pragma acc parallel async(4)
#pragma acc loop
#pragma acc wait(4)
