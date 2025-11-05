#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[:n])
#pragma acc parallel
#pragma acc loop
