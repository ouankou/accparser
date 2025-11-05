#pragma acc enter data copyin(a[0:10*n], b[0:10*n], d[0:10])
#pragma acc serial private(c[0:n])
#pragma acc loop gang
#pragma acc loop worker
#pragma acc loop seq
#pragma acc exit data copyout(d[0:10]) delete(a[0:10*n], b[0:10*n])
