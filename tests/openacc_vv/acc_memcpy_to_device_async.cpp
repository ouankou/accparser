#pragma acc data deviceptr(devdata)
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc parallel async(4)
#pragma acc loop
#pragma acc parallel async(5)
#pragma acc loop
#pragma acc parallel async(3) wait(1, 2)
#pragma acc loop
#pragma acc parallel async(6) wait(4, 5)
#pragma acc loop
#pragma acc update host(hostdata[2*n:n]) async(3)
#pragma acc update host(hostdata[5*n:n]) async(6)
#pragma acc wait(3)
#pragma acc wait(6)
#pragma acc exit data delete(hostdata[0:6*n])
