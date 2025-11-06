#pragma acc data copyin(a[0:length], b[0:length]) copyout(c[0:length])
#pragma acc parallel
#pragma acc loop
