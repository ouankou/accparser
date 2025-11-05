#pragma acc parallel loop copyout(test) copy(test) reduction(+:test)
#pragma acc parallel loop copyout(test[0:n]) copy(test[0:n])
#pragma acc parallel loop copy(test) copyout(test) reduction(+:test)
#pragma acc parallel loop copy(test[0:n]) copyout(test[0:n])
