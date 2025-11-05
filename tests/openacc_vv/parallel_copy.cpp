#pragma acc parallel copy(a[0:n])
#pragma acc loop
#pragma acc parallel loop copy(device) reduction(+:device)
