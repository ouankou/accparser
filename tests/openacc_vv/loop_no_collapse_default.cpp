#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(c[0:10*n])
#pragma acc parallel
#pragma acc loop independent
