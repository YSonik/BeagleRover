#ifndef __GYROSCOPE_H
#define __GYROSCOPE_H

void Gyroscope_init(void);
void Gyroscope_cleanUp(void);

int Gyroscope_getAngle(int16_t *zGy);
int Gyroscope_getDirection(void);

#endif // __GYROSCOPE_H