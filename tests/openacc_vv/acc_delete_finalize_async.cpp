#pragma acc enter data copyin(a[0:n], b[0:n], d[0:n], e[0:n])
#pragma acc enter data copyin(a[0:n], b[0:n], d[0:n], e[0:n])
#pragma acc data present(a[0:n], b[0:n], d[0:n], e[0:n]) copyout(c[0:n], f[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc wait
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data copyin(a[0:n], b[0:n], c[0:n])
#pragma acc enter data copyin(c[0:n])
#pragma acc data present(a[0:n], b[0:n], c[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc enter data copyin(c[0:n]) async(1)
#pragma acc data present(a[0:n], b[0:n], c[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n]) copyout(c[0:n])
