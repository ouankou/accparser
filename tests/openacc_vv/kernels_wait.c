#pragma acc enter data copyin(a[0:n], b[0:n], c[0:n]) async(1)
#pragma acc kernels wait(1)
#pragma acc loop
#pragma acc exit data copyout(c[0:n]) delete(a[0:n], b[0:n])
