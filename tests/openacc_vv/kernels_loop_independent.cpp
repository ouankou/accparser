#pragma acc data copyin(a[0:n]) copyout(b[0:n])
#pragma acc kernels loop independent
