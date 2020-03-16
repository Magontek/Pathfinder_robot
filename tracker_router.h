#define DIV 19

/* Este objeto deberia resolver 3 cosas
 *  Existen obstaculos?
 *  |-> que forma tienen?
 *  Puedo esquivarlo?
 *  |-> margen de error
 *  |-> recalculo al avanzar
 */
class tracker_router{
  private:
    int _v_min_real;
    int _v_max_real;
    float _d_ejes;
    int* _v_actual[2];
    int _v_obj[2];
    int _paredes[4][2];
  public:
    tracker_router(float d_ejes, int& v_actual_m1, int& v_actual_m2);
    void set_max_min_real(int max_real, int min_real);
    byte curva(int velocidad, int radio, boolean permisivo);
    byte paredes();
    byte esquivar_pared();
    byte hay_paso();
};

tracker_router::tracker_router(float d_ejes, int& v_actual_m1, int& v_actual_m2){
  _v_min_real=0;
  _v_max_real=10;
  _d_ejes=1;
  _v_actual[0]=&v_actual_m1;
  _v_actual[1]=&v_actual_m2;
  _v_obj[0]=0;
  _v_obj[1]=0;
}

//estos limites de velocidad son para los calculos de curvas y radios posibles de giro
void tracker_router::set_max_min_real(int max_real, int min_real){
    _v_min_real=min_real;
    _v_max_real=max_real;
}

//calcula si es posible generar una curva y devuelve valores de las velocidades objetivo de los motores
byte tracker_router::curva(int velocidad, int radio, boolean permisivo){
  Serial.print(permisivo);
  Serial.print(" ");
  int medio = _d_ejes/2;
  int v_2, v_1;
  v_1 = velocidad;
  Serial.print(v_1);
  Serial.print(" ");
  if(v_1<_v_min_real){
    if(permisivo){
      v_1=_v_min_real;
    }
    else{
      return 1;
    }
  }
  v_2 = v_1*(radio+medio)/(radio-medio);
  if (v_2>_v_max_real&&permisivo){
    if(permisivo){
      v_2=_v_max_real;
      v_1=v_2*(radio-medio)/(radio+medio);
      if(v_1<_v_min_real){
        return 3;
      }
    }
    else{
      return 2;
    }
  }
  _v_obj[0]=v_1;
  _v_obj[1]=v_2;
  return -1;
}

//Verifica si las mediciones son compatibles con paredes
//Deberia poder responder a diferentes situaciones y devolver un mapa de rectas de entre 1 y 4
byte tracker_router::pared(){
  bool hay=0;
  int cant_puntos=0;
  for(int i=0;i<DIV;i++){
   
  }
  if(cant_puntos>1){
    return(hay);
  }
  else{
    return(0);
  }
}
