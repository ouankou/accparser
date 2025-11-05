#pragma acc data copyin(a[0:n], b[0:n]) present(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc update host(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n]) present(c[0:n], e[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc parallel
#pragma acc loop
#pragma acc update host(c[0:n])
#pragma acc update host(e[0:n])
#pragma acc data copyin(a[0:n], b[0:n]) deviceptr(d)
#pragma acc parallel
#pragma acc loop
#pragma acc data copyin(a[0:n], b[0:n]) present(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc update host(c[0:n])
