#pragma acc enter data create(a[0:n], b[0:n], c[0:n])
#pragma acc data present(a[0:n], b[0:n], c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n], c[0:n])
#pragma acc exit data delete(hostdata[0:3*n])
