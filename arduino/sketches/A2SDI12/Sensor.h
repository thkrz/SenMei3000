#ifndef _SENSOR_H
#define _SENSOR_H

class Sensor {
  private:
    int n;
    int pin[2];
    float v[2];
  public:
  virtual void data(int n);
  virtual void ident();
  void measure();
  virtual void prepare();
}

class SMT100: public Sensor {
}

#endif /* _SENSOR_H */
