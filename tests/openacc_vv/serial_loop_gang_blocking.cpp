#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc serial
#pragma acc loop gang
#pragma acc loop gang
