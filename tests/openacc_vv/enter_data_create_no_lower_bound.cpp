#pragma acc enter data copyin(a[0:n], b[0:n]) create(c[:n])
#pragma acc parallel present(a[0:n], b[0:n], c[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n]) copyout(c[0:n])
