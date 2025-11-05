#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data copyin(a[0:n])
#pragma acc parallel present(a[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n])
#pragma acc enter data copyin(a[0:n])
#pragma acc parallel present(a[0:n])
#pragma acc loop
#pragma acc exit data copyout(a[0:n])
