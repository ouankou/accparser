#pragma acc enter data copyin(a[0:n], b[0:n]) create(c[0:n]) async(1)
#pragma acc enter data copyin(d[0:n]) create(e[0:n]) async(2)
#pragma acc parallel present(a[0:n], b[0:n], c[0:n]) async(1)
#pragma acc loop
#pragma acc parallel present(c[0:n], d[0:n], e[0:n]) async(1) wait(2)
#pragma acc loop
#pragma acc exit data copyout(e[0:n]) async(1)
#pragma acc data copyin(a[0:n], b[0:n], d[0:n]) create(c[0:n]) copyout(e[0:n])
#pragma acc parallel present(a[0:n], b[0:n], c[0:n]) async(1)
#pragma acc loop
#pragma acc parallel present(c[0:n], d[0:n], e[0:n]) async(1)
#pragma acc loop
#pragma acc data copyin(a[0:n], b[0:n], d[0:n]) create(c[0:n]) copyout(e[0:n])
#pragma acc parallel present(a[0:n], b[0:n], c[0:n]) async
#pragma acc loop
#pragma acc parallel present(c[0:n], d[0:n], e[0:n]) async
#pragma acc loop
