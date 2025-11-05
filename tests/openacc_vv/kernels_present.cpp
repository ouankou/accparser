#pragma acc enter data copyin(a[0:n]) create(b[0:n])
#pragma acc kernels present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data copyout(b[0:n]) delete(a[0:n])
