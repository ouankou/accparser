#pragma acc enter data copyin(a[0:n], b[0:n])
#pragma acc data copyout(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc data copyout(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n])
#pragma acc loop
