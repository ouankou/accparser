#pragma acc kernels if(data_on_device) present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc enter data copyin(a[0:n]) create(b[0:n])
#pragma acc kernels if(data_on_device) present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data copyout(b[0:n]) delete(a[0:n])
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data copyin(a[0:n]) create(b[0:n])
#pragma acc kernels if(data_on_device) present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n])
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data copyin(a[0:n], b[0:n])
#pragma acc kernels if(data_on_device) present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n])
