#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n]) create(c[0:n], f[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc update host(c[0:n], f[0:n]) wait(1, 2) if(true)
#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n]) create(c[0:n], f[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc update host(c[0:n], f[0:n]) wait(1) if(true)
#pragma acc update host(c[0:n], f[0:n]) wait(2) if(true)
#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n]) create(c[0:n], f[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc update host(c[0:n], f[0:n]) wait(1, 2) if(false)
#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n]) create(c[0:n], f[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc update host(c[0:n], f[0:n]) wait(1) if(false)
#pragma acc update host(c[0:n], f[0:n]) wait(2) if(false)
