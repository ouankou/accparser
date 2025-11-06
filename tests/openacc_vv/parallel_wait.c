#pragma acc enter data create(a[0:n])
#pragma acc update device(a[0:n]) async(1)
#pragma acc parallel present(a[0:n]) wait(1)
#pragma acc loop
#pragma acc exit data copyout(a[0:n])
