#pragma acc serial default(none) reduction(+:temp)
#pragma acc serial loop reduction(+:temp)
#pragma acc serial reduction(+:device)
#pragma acc serial loop
