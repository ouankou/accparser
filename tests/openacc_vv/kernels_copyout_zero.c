#pragma acc data copyin(a[0:n])
#pragma acc kernels copyout(zero: b[0:n])
#pragma acc loop
#pragma acc enter data copyin(devtest[0:1])
#pragma acc kernels present(devtest[0:1])
#pragma acc data copyin(a[0:n])
#pragma acc kernels copyout(zero: b[0:n])
#pragma acc loop
