#pragma acc data copy(b[0:n])
#pragma acc kernels copyin(a[0:n])
#pragma acc loop
#pragma acc enter data copyin(devtest[0:1])
#pragma acc kernels present(devtest[0:1])
#pragma acc data copy(a[0:n], b[0:n])
#pragma acc kernels copyin(a[0:n])
#pragma acc loop
#pragma acc enter data copyin(devtest[0:1])
#pragma acc kernels present(devtest[0:1])
#pragma acc kernels copyin(a[0:n], b[0:n])
#pragma acc loop
