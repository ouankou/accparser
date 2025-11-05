#pragma acc enter data create(c[0:n], f[0:n])
#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n])
#pragma acc parallel async(1) present(c[0:n])
#pragma acc loop
#pragma acc parallel async(2) present(f[0:n])
#pragma acc loop
#pragma acc wait
#pragma acc enter data create(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel present(c[0:n]) async
#pragma acc loop
#pragma acc wait
#pragma acc enter data create(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel present(c[0:n]) async
#pragma acc loop
#pragma acc wait
#pragma acc enter data create(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel present(c[0:n]) async(1)
#pragma acc loop
#pragma acc enter data copyin(c[0:n])
#pragma acc parallel present(c[0:n]) async(1)
#pragma acc loop
