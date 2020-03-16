
class motor_driver{
  private:
    int _v_min_real;
    int _v_max_real;
    byte _m_a[2]; //pines de motores
    byte _m_b[2];
    int _v_min;
    int _v_max;
    int _v_offset;
    int _v_obj[2];
    int* _v_actual[2];
    int _output_a_pwm[2];
    float _kp, _ki, _kd;
    int _error_previo[2];
    unsigned long _lastStep;
    unsigned long _dT;
    int _integral[2];

    void pid(bool selector);
  public:
    motor_driver(const uint8 m1_a, const uint8 m1_b, const uint8 m2_a,const uint8 m2_b, int& v_actual_m1, int& v_actual_m2);
    void set_pid(float kp,float ki,float kd, int v_min, int v_max, int v_offset);
    void derecho(byte velocidad);
    void actualizar();
    void print_data(int opt);
    void set_velocidad(int velocidad, int motor);
};

motor_driver::motor_driver(const uint8 m1_a, const uint8 m1_b, const uint8 m2_a,const uint8 m2_b, int& v_actual_m1, int& v_actual_m2){
  _m_a[0]=m1_a;
  _m_b[0]=m1_b;
  _m_a[1]=m2_a;
  _m_b[1]=m2_b;
  _v_min=0;
  _v_max=100;
  _v_obj[0]=0;
  _v_obj[1]=0;
  _v_actual[0]=&v_actual_m1;
  _v_actual[1]=&v_actual_m2;
  _kp=1;
  _ki=0;
  _kd=0;
}

void motor_driver::set_pid(float kp,float ki,float kd, int v_min, int v_max, int v_offset){
  _kp=kp;
  _ki=ki;
  _kd=kd;
  _v_min=v_min;
  _v_max=v_max;
  _v_offset=v_offset;
}

void motor_driver::set_velocidad(int velocidad, int motor){
  _v_obj[motor]=velocidad;
}

void motor_driver::pid(bool selector) {

  //calculo del error lineal
  int _error = abs(_v_obj[selector]) - *_v_actual[selector];
  //Calculo integrativo por integral de riemman
  _integral[selector] += int(float(_error + _error_previo[selector]) / float(2 * _dT / 1000.0));
  //Calculo derivativo por aproximacion
  int _dError = int((_error - _error_previo[selector]) / float(_dT) / 1000.0);

      /*Serial.print(_error);
      Serial.print(" ");
      Serial.print(_integral[selector]);
      Serial.print(" ");
      Serial.print(_dError);
      Serial.print(" ");
      Serial.print(_error_previo[selector]);
      Serial.print(" ");
      */
  
  _error_previo[selector] = _error;
  //Suma de los elementos pid
  int PID = (_kp * _error) + (_ki * _integral[selector]) + (_kd * _dError);
  _output_a_pwm[selector] = constrain(PID, _v_min, _v_max);
/*
      Serial.print(PID);
      Serial.print(" ");
      Serial.print(_output_a_pwm[selector]);
      Serial.println(" ");*/
}

void motor_driver::derecho(byte velocidad){
  _v_obj[0]=velocidad;
  _v_obj[1]=velocidad;
}


void motor_driver::actualizar(){
  _dT = millis() - _lastStep; //esto deberia ir adentro de pid pero solo se calcula 1 vez
  _lastStep = millis();
  for(byte i=0;i<=1;i++){
    if(_v_obj[i] != *_v_actual[i]){
      pid(i);
      if (_v_obj[i]>0){
        pwmWrite(_m_a[i],_output_a_pwm[i]+_v_offset);
        pwmWrite(_m_b[i],0);
      }
      else if(_v_obj[i]<0){
        pwmWrite(_m_a[i],0);
        pwmWrite(_m_b[i],_output_a_pwm[i]+_v_offset);
      }
      else{
        _integral[i]=0;
        pwmWrite(_m_a[i],0);
        pwmWrite(_m_b[i],0);
      }
    }
    
  }
}

void motor_driver::print_data(int opt){
  switch (opt){
    case 0:
      for(byte i=0;i<=1;i++){
        Serial.print("M");
        Serial.print(i);
        Serial.print(": v_obj:");
        Serial.print(_v_obj[i]);
        Serial.print(" v_act:");
        Serial.print(*_v_actual[i]);
        Serial.print(" pwm:");
        Serial.print(_output_a_pwm[i]);
        Serial.print(" err:");
        Serial.print(_error_previo[i]);
        Serial.print(" int:");
        Serial.print(_integral[i]);
        Serial.print(" lstep:");
        Serial.print(_lastStep);
        Serial.print(" dT:");
        Serial.print(_dT);
        Serial.println(" ");
      }
    break;
    case 1:
        Serial.print("p:");
        Serial.print(_kp);
        Serial.print(" i:");
        Serial.print(_ki);
        Serial.print(" d:");
        Serial.print(_kd);
        Serial.println(" ");
    break;
    case 2:
      Serial.print("M1: v_obj:");
      Serial.print(_v_obj[0]);
      Serial.print(" M2: v_obj:");
      Serial.println(_v_obj[1]);
    break;
    case 3:
      Serial.print("v_min_real:");
      Serial.print(_v_min_real);
      Serial.print(" v_max_real:");
      Serial.println(_v_max_real);
  }
}
