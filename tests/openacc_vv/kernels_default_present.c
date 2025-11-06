#pragma acc enter data copyin(a[0:n]), create(b[0:n])
#pragma acc kernels default(present)
#pragma acc loop
#pragma acc exit data delete(a[0:n]), copyout(b[0:n])
