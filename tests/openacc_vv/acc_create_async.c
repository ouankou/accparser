#pragma acc data copyin(a[0:n], b[0:n], d[0:n], e[0:n])
#pragma acc data present(c[0:n], f[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc wait
#pragma acc exit data copyout(c[0:n], f[0:n])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel present(c[0:n])
#pragma acc loop
#pragma acc exit data copyout(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc wait
#pragma acc parallel present(c[0:n])
#pragma acc loop
#pragma acc exit data copyout(c[0:n])
#pragma acc enter data create(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n], d[0:n]) copyout(e[0:n])
#pragma acc parallel present(c[0:n]) async(1)
#pragma acc loop
#pragma acc parallel present(c[0:n]) async(1)
#pragma acc loop
#pragma acc wait
#pragma acc exit data copyout(c[0:n])
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data create(c[0:n])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel present(c[0:n]) async(1)
#pragma acc loop
#pragma acc parallel present(c[0:n]) async(1)
#pragma acc loop
#pragma acc exit data copyout(c[0:n]) async(1)
#pragma acc wait
