#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc serial
#pragma acc loop
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data copyin(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc serial
#pragma acc loop
#pragma acc exit data copyout(c[0:n])
