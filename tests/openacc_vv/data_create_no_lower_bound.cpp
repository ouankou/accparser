#pragma acc data copyin(a[0:n], b[0:n], d[0:n]) create(c[:n]) copyout(e[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc loop
