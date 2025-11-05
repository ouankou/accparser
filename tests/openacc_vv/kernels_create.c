#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc data copyin(a[0:n])
#pragma acc kernels create(b[0:n])
#pragma acc loop
#pragma acc data copyin(a[0:n]) copyout(b[0:n])
#pragma acc kernels create(b[0:n])
#pragma acc loop
#pragma acc data copyin(a[0:n]) copyout(c[0:n])
#pragma acc kernels create(b[0:n])
#pragma acc loop
#pragma acc loop
