
class motor_driver{
  private:
    uint8 _m_a[2]; //pines de motores
    uint8 _m_b[2];
    int _default_aceleracion; //Aceleracion default
    int _v_min;
    int _v_max;
    int _v_target[2]; //Velocidad objetivo
    int _v_paso[2]; //Incremento de velocidad cada milisegundo
    int _v_actual[2]; //velocidad actual
    bool _actualizar; //Indica si es necesario actualiza la velocidad
    float _d_ejes;
    AutoPID *pid_M[2];
  public:
    motor_driver(const uint8 m1_a, const uint8 m1_b, const uint8 m2_a,const uint8 m2_b, int default_aceleracion, float d_ejes, AutoPID &M1, AutoPID &M2);
    void derecho(int velocidad, int aceleracion);
    void derecho(int velocidad);
    uint8 curva(int velocidad, int radio, int aceleracion);
    uint8 curva(int velocidad, int radio);
    void actualizar();
};

motor_driver::motor_driver(const uint8 m1_a, const uint8 m1_b, const uint8 m2_a,const uint8 m2_b, int default_aceleracion, float d_ejes, AutoPID &M1, AutoPID &M2){
  _m_a[0]=m1_a;
  _m_b[0]=m1_b;
  _m_a[1]=m2_a;
  _m_b[1]=m2_b;
  _default_aceleracion=default_aceleracion;
  _v_target[0]=0;
  _v_target[1]=0;
  _v_paso[0]=0;
  _v_paso[1]=0;
  _v_actual[0]=0;
  _v_actual[1]=0;
  _actualizar=0;
  _v_min=0;
  _v_max=100;
  _d_ejes=d_ejes;
  pid_M[0]=&M1;
  pid_M[1]=&M2;
}

//Este deberia actualizase con el timer cada 1ms
void motor_driver::actualizar(){
  for (int i=0;i<=1;i++){
       
    //Actualizo los pwm de los motores
    if (_actualizar){
      if (_v_actual[i]>0){
        pwmWrite(_m_a[i],_v_actual[i]*2);
        pwmWrite(_m_b[i],0);
        _actualizar=false;
        Serial.print("velocidad +: ");
        Serial.print(_m_a[i]);
        Serial.print(" ");
        Serial.println(_v_actual[i]*2);
      }
      else if(_v_actual[i]<0){
        pwmWrite(_m_b[i],_v_actual[i]*2);
        pwmWrite(_m_a[i],0);
        _actualizar=false;
        Serial.println("velocidad -: ");
      }
      else{
        
      }
    }
  }
}

void motor_driver::derecho(int velocidad){
  this->derecho(velocidad,_default_aceleracion);
}

void motor_driver::derecho(int velocidad, int aceleracion){
  for (int i=0;i<=1;i++){
    if (_v_target[i]<velocidad){
      _v_paso[i]=aceleracion;
    }
    else{
      _v_paso[i]=-aceleracion;
    }
    if(velocidad==0){
      _v_target[i]=0;
    }
    else if(velocidad<_v_min){
      _v_target[i]=_v_min;
    }
    else if(velocidad>32767){
      _v_target[i]=32767;
    }
    else{
      _v_target[i]=velocidad;
    }
    
    Serial.print("Derecho ");
    Serial.print(_v_target[i]);
    Serial.println(_v_paso[i]);
  }
}

uint8 motor_driver::curva(int velocidad, int radio, int aceleracion){
  float d=(radio-_d_ejes)/(radio+_d_ejes);
  Serial.print("D: ");
  Serial.print(d);
  float dn[2];
  dn[0]=(radio-_d_ejes)/radio;
  Serial.print(" Dn1: ");
  Serial.print(dn[0]);
  dn[1]=(radio+_d_ejes)/radio;
  Serial.print(" Dn2: ");
  Serial.println(dn[1]);
  int v_local[2];
  uint8 state;
  bool lado=0;
  for (int i=0;i<=1;i++){
    v_local[i]=velocidad*dn[i];
    Serial.print("V");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(v_local[i]);
    if(v_local[i]<_v_min){
      v_local[lado]=_v_min;
      v_local[!lado]=_v_min*d;
      if(v_local[!lado]>_v_max){
        v_local[lado]=0;
        v_local[!lado]=0;
        state=1;
        return(state);
      }
    }
    if(v_local[lado]>_v_max){
      v_local[lado]=0;
      v_local[!lado]=0;
      state=2;
      return(state);
    }
    
    lado=!lado;
  }
  _v_target[0]=v_local[0];
  _v_target[1]=v_local[1];
  if(v_local[0]>v_local[1]){
    _v_paso[0]=aceleracion;
    _v_paso[1]=(_v_target[1]-_v_actual[1])/((_v_target[0]-_v_actual[0])/aceleracion);
  }
  else{
    _v_paso[1]=aceleracion;
    _v_paso[0]=(_v_target[0]-_v_actual[0])/((_v_target[1]-_v_actual[1])/aceleracion);
  }
  Serial.print("Acel 1: ");
  Serial.print(_v_paso[0]);
  Serial.print("Acel 2: ");
  Serial.print(_v_paso[1]);
  state=0;
  return(state);
}
