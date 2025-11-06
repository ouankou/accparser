#pragma acc enter data create(b[0:n])
#pragma acc data copyin(a[0:n])
#pragma acc parallel present(b[0:n])
#pragma acc loop
#pragma acc data copyout(c[0:n])
#pragma acc parallel present(b[0:n])
#pragma acc loop
#pragma acc exit data delete(b[0:n])
#pragma acc enter data present_or_create(b[0:n])
#pragma acc data copyin(a[0:n])
#pragma acc parallel present(b[0:n])
#pragma acc loop
#pragma acc data copyout(c[0:n])
#pragma acc parallel present(b[0:n])
#pragma acc loop
#pragma acc exit data delete(b[0:n])
#pragma acc enter data pcreate(b[0:n])
#pragma acc data copyin(a[0:n])
#pragma acc parallel present(b[0:n])
#pragma acc loop
#pragma acc data copyout(c[0:n])
#pragma acc parallel present(b[0:n])
#pragma acc loop
#pragma acc exit data delete(b[0:n])
