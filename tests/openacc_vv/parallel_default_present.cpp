#pragma acc enter data copyin(a[0:n])
#pragma acc parallel default(present)
#pragma acc loop
#pragma acc exit data copyout(a[0:n])
