#pragma acc data deviceptr(devdata)
#pragma acc parallel
#pragma acc loop
#pragma acc exit data delete(hostdata[0:3*n])
