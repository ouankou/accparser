#pragma acc enter data copyin(a[0:n], b[0:n]) if(dev)
#pragma acc data create(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n]) if(dev)
#pragma acc enter data create(a[0:n], b[0:n]) if(host)
#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data delete(a[0:n], b[0:n]) if(host)
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel
#pragma acc enter data copyin(a[0:n], b[0:n]) if(dev)
#pragma acc data create(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n]) if(dev)
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel
#pragma acc enter data copyin(a[0:n], b[0:n]) if(host)
#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n]) if(host)
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel
#pragma acc enter data create(a[0:n], b[0:n]) if(host)
#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data copyout(a[0:n], b[0:n]) if(host)
