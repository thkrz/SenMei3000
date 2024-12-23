#ifndef _SENSOR_H
#define _SENSOR_H

class Sensor {
  virtual void readSample();
}

class SMT100: public Sensor {
  private:
    int pin[2];

  public:

}

#endif /* _SENSOR_H */
