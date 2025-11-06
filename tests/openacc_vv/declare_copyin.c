#pragma acc declare copyin(mult_copyin)
#pragma acc routine vector
#pragma acc loop
#pragma acc declare copyin(fixed_size_array)
#pragma acc declare copyin(scalar)
#pragma acc declare copyin(datapointer)
#pragma acc declare copyin(n)
#pragma acc routine vector
#pragma acc loop vector
#pragma acc data copyin(a[0:n]) copyout(b[0:n]) present(fixed_size_array)
#pragma acc parallel
#pragma acc loop
#pragma acc data copyin(a[0:n]) copyout(b[0:n]) present(scalar)
#pragma acc parallel
#pragma acc loop
#pragma acc data copy(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data copy(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc enter data copyin(a[0:n]) attach(datapointer)
#pragma acc data present(datapointer[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc exit data copyout(a[0:n])
