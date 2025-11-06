#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data create(a[0:n])
#pragma acc exit data delete(a[0:n])
