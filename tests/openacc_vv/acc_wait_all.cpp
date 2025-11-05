#pragma acc data copyin(a[0:n], b[0:n], c[0:n]) copyout(d[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc parallel async(3)
#pragma acc loop
#pragma acc parallel
#pragma acc loop
