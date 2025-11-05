#pragma acc parallel copyin(a[0:n]) reduction(+:reduction)
#pragma acc loop independent
