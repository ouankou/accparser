#pragma acc parallel loop copyin(test) copyout(test) reduction(+:test)
#pragma acc parallel loop copyin(test[0:n]) copyout(test[0:n])
