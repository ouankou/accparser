#pragma acc enter data copyin(a[0:n])
#pragma acc parallel deviceptr(b)
#pragma acc loop
#pragma acc exit data copyout(a[0:n])
