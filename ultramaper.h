#include <Servo.h>
#include <NewPing.h>

#define DIV 19

class Ultramaper {
  private:
    NewPing *_sonar;
    Servo *_servo;
    byte _last_angle;
    bool _direcion;
    byte _last_count;
    double _last_time;
    byte _divisiones;
    bool _oktr;
  public:
    byte lastread; //ultima lectura en cm
    byte mapa[DIV]; //este solo funciona si se usan 20 divisiones
    byte scanNext(bool make_delay, bool force);
    Ultramaper();
    void atachSonar(NewPing &sonar);
    void atachServo(Servo &servo);
    void configuracion(byte divisiones);
    void mapeo(byte a_min, byte a_max);
    void mapeo();
    byte hay_pared();
    byte last_read();

};

Ultramaper::Ultramaper(){
  _last_angle=0;
  _direcion=0;
  lastread=0;
  mapa[19]=0;
  _last_count=0;
  _last_time=0;
  _divisiones=18;
  _oktr=true;
}


void Ultramaper::configuracion(byte divisiones){
  _divisiones=divisiones;
}

byte Ultramaper::last_read(){
  if((_last_time-millis())>=(450/_divisiones)){
    lastread=byte(_sonar->ping_cm());
    mapa[_last_count]=lastread;
    _oktr=true;
    return(lastread);
  }
  else{
    _oktr=false;
    return(0);
  }
}

byte Ultramaper::scanNext(bool make_delay, bool force){
  if(_oktr || force){
    if (_direcion){
      _last_count--;
    }
    else{
      _last_count++;
    }
    _last_angle=_last_count*10;
    if (_last_angle>=180 || _last_angle<=0){
      _direcion=!_direcion;
    }
  
    _servo->write(_last_angle);
    _last_time=millis();
    if(make_delay){
      delay(int(450/_divisiones));
      lastread=byte(_sonar->ping_cm());
      mapa[_last_count]=lastread;
      return (lastread);
      _oktr=true;
    }
    else{
      _oktr=false;
      return(0);
    } 
  }
  else{
    return(-1);
  }
}

void Ultramaper::mapeo(byte a_min, byte a_max){
  _last_angle=a_min;
  int paso_final=(a_max-a_min)/180/_divisiones;
  for(int i=0;i<paso_final;i++){
    scanNext(true, true);
  }
}

void Ultramaper::mapeo(){
  mapeo(0, 180);
}

void Ultramaper::atachSonar(NewPing &sonar){
  _sonar=&sonar;
}

void Ultramaper::atachServo(Servo &servo){
  _servo=&servo;
}
