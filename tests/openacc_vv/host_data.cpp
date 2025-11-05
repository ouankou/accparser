#pragma acc enter data copyin(a[0:n])
#pragma acc host_data use_device(a)
#pragma acc enter data copyin(a_points[0:n])
#pragma acc parallel present(a[0:n], a_points[0:n])
#pragma acc loop
#pragma acc exit data delete(a_points[0:n]) copyout(a[0:n])
