#pragma acc data copy(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc data copy(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n])
#pragma acc enter data copyin(dev_test[0:1])
#pragma acc parallel present(dev_test[0:1])
#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data copy(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n])
#pragma acc data copyout(c[0:n]) present(a[0:n], b[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n])
#pragma acc data copyout(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n])
#pragma acc exit data delete(a[0:n], b[0:n])
#pragma acc enter data copyin(dev_test[0:1])
#pragma acc parallel present(dev_test[0:1])
#pragma acc data copyout(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n])
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n])
#pragma acc exit data delete(a[0:n], b[0:n])
