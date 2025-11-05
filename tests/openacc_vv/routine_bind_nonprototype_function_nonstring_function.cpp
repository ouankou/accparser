#pragma acc routine vector bind(device_array_array)
#pragma acc loop reduction(+:returned)
#pragma acc loop reduction(-:returned)
#pragma acc routine vector bind(device_object_array)
#pragma acc loop reduction(-:returned)
#pragma acc loop reduction(-:returned)
#pragma acc routine vector bind(device_array_object)
#pragma acc loop reduction(+:returned)
#pragma acc loop reduction(-:returned)
#pragma acc routine vector bind(device_object_object)
#pragma acc loop reduction(-:returned)
#pragma acc loop reduction(-:returned)
#pragma acc data copyin(a[0:n]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
#pragma acc data copyin(a, a.data[0:n]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
#pragma acc data copyin(a[0:n], b, b.data[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
#pragma acc update host(b.data[0:n])
#pragma acc data copyin(a, a.data[0:n], b ,b.data[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
#pragma acc update host(b.data[0:n])
