#pragma acc enter data copyin(a[0:n], b[0:n]) create(c[0:n])
#pragma acc data deviceptr(a_ptr, b_ptr, c_ptr)
#pragma acc parallel
#pragma acc loop
#pragma acc exit data copyout(c[0:n]) delete(a[0:n], b[0:n])
