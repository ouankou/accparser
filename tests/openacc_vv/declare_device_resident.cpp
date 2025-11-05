#pragma acc declare device_resident(mult_device_resident)
#pragma acc routine vector
#pragma acc loop
#pragma acc declare device_resident(fixed_size_array)
#pragma acc declare device_resident(scalar)
#pragma acc declare device_resident(datapointer)
#pragma acc parallel
#pragma acc data copyin(a[0:n]) copyout(b[0:n]) present(fixed_size_array)
#pragma acc parallel
#pragma acc loop
#pragma acc parallel
#pragma acc loop
#pragma acc parallel
#pragma acc data copyin(a[0:n]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc parallel
#pragma acc data copy(a[0:n])
#pragma acc parallel
#pragma acc loop
