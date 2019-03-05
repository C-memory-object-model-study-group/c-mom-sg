#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include <inttypes.h>
int y=2, x=1;
int main() {
  int *p = &x+1;
  int *q = &y;
  if (memcmp(&p, &q, sizeof(p)) == 0) {
    uintptr_t i = (uintptr_t)p;
    int *r = (int *)i;
    r=r-1;  // is this free of UB?
    *r=11;  // and this?    
    printf("x=%d y=%d *p=%d *q=%d *r=%d\n",x,y,*p,*q,*r); 
  }
}
