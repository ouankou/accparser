#pragma acc set device_num(x)
#pragma acc enter data copyin(a[0:n])
#pragma acc set device_num(x)
#pragma acc data present(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc set device_num(x)
#pragma acc exit data copyout(a[0:n])
