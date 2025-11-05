#pragma acc serial if(host)
#pragma acc loop
#pragma acc data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc enter data copyin(a[0:n], b[0:n], c[0:n])
#pragma acc serial if(host) present(a[0:n], b[0:n], c[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc enter data copyin(a[0:n], b[0:n], c[0:n])
#pragma acc serial if(accel) present(a[0:n], b[0:n], c[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n]) copyout(c[0:n])
