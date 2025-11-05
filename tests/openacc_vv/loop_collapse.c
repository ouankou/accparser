#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(c[0:10*n])
#pragma acc parallel
#pragma acc loop independent collapse(1)
#pragma acc data copyin(a[0:10*n], b[0:10*n]) copyout(c[0:10*n])
#pragma acc parallel
#pragma acc loop independent collapse(2)
