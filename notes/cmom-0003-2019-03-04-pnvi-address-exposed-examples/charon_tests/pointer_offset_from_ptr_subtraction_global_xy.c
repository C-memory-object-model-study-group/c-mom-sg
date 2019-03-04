#include <stdio.h>
#include <string.h> 
#include <stddef.h>
int x=1, y=2;
int main() {
  int *p = &x;
  int *q = &y;
  ptrdiff_t offset = q - p;
  int *r = p + offset;
  if (memcmp(&r, &q, sizeof(r)) == 0) {
    *r = 11; // is this free of UB?
    printf("y=%d *q=%d *r=%d\n",y,*q,*r); 
  }
}
