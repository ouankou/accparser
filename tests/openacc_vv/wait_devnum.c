#pragma acc data copy(a[0:n], b[0:n])
#pragma acc parallel loop async(1)
#pragma acc wait(devnum:0 : queues:1) async(2)
#pragma acc parallel loop async(2)
#pragma acc wait
