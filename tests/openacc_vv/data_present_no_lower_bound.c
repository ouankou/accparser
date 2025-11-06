#pragma acc enter data copyin(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n]) present(c[:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data copyout(c[0:n])
