#pragma acc routine(host_function_identifier_named) vector bind(device_function_identifier_named)
#pragma acc routine(host_function_string_named) vector bind("device_function_string_named")
#pragma acc routine vector bind(device_function_identifier_unnamed)
#pragma acc loop reduction(-:returned)
#pragma acc routine vector bind("device_function_string_unnamed")
#pragma acc loop reduction(-:returned)
#pragma acc loop reduction(-:returned)
#pragma acc loop reduction(-:returned)
#pragma acc data copyin(a[0:n][0:n]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
#pragma acc data copyin(a[0:n][0:n]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
#pragma acc data copyin(a[0:n][0:n]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
#pragma acc data copyin(a[0:n][0:n]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop gang worker vector
