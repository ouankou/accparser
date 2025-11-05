#pragma acc serial copyin(a[0:n]) reduction(+:reduction)
#pragma acc loop
