#pragma acc data copyin(a[:n], b[:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
