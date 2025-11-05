#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel
#pragma acc enter data copyin(a[0:n], b[0:n], c[0:n])
#pragma acc enter data create(a[0:n], b[0:n], c[0:n])
#pragma acc parallel present(a[0:n], b[0:n], c[0:n])
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n], c[0:n])
#pragma acc exit data copyout(c[0:n]) delete(a[0:n], b[0:n])
#pragma acc enter data copyin(a[0:n], b[0:n], c[0:n])
#pragma acc enter data copyin(a[0:n], b[0:n], c[0:n])
#pragma acc parallel present(a[0:n], b[0:n], c[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n]) copyout(c[0:n]) finalize
