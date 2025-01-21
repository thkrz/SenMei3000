#ifndef _SMT100_H
#define _SMT100_H

#define INFO "a13TRUEBNERSMT100038241127102256"
#define WAIT "0022"

float smt100moist(float);
float smt100temp(float);

extern float (*tr[2])(float);

#endif /* _SMT100_H */
