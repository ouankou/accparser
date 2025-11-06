#pragma acc data copyin(a[0:10*n], b[0:10*n]) copyout(c[0:10*n])
#pragma acc parallel
#pragma acc loop collapse(force:2)
