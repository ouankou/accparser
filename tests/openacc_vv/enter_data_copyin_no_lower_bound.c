#pragma acc enter data copyin(a[:n], b[:n]) create(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n], c[0:n])
#pragma acc loop
#pragma acc exit data copyout(c[0:n]) delete(a[0:n], b[0:n])
