#pragma acc data copyin(list[0:3][0:n])
#pragma acc enter data copyin(list[i]) async(i)
#pragma acc parallel loop async(i)
#pragma acc kernels
