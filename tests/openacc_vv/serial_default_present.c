#pragma acc enter data copyin(a[0:n])
#pragma acc serial default(present)
#pragma acc loop
#pragma acc exit data copyout(a[0:n])
