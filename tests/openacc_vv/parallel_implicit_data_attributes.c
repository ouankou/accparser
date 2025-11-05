#pragma acc parallel default(none) reduction(+:test)
#pragma acc parallel loop reduction(+:a)
#pragma acc parallel reduction(+:device_value)
#pragma acc parallel loop
