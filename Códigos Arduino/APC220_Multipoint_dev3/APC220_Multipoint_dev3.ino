const byte SetPin = 13; //Pin para poner el APC220 en modo de configuración
volatile unsigned long last_micros; //Variable para guargar el valor de la ultima lectura de tiempo
long debouncing_time = 70; //Tiempo de espera para evitar los rebotes
//Red - 450MHz
//Dispositivo 4
#define my_addr 0x30 //Identificacion propia del dispositivo
#define addr1 0x10
#define addr2 0x20
#define addr3 0x40
 
bool send_dev1=false,send_dev2=false,send_dev3=false; //Variables bandera para el envio de datos a los dispositivos
//Definicion de los pines a usar como salida de LEDs
int LED1 = 5, LED2 = 6, LED3 = 7;

void setup() {
  Serial.begin(57600);  //Solo para depuracion
  Serial1.begin(57600);
  pinMode(SetPin, OUTPUT); //Configuracion de pin de control para el modulo como salida
  //Configuracion de pines de interrupcion, sin la resistencia PULLUP en el DUE
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(20, INPUT_PULLUP);

  //Configuracion de pines para los LED como salidas
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  //Configuracion de pines para lectura de los switch con la activacion de la resistencia PULLUP
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  
  digitalWrite(SetPin, HIGH);//Inicializacion del pin de control SET en alto
  ConfigAPC(); //Configuracion del modulo
  delay(100);    //Espera
  ConfigAPC(); /* Segunda configuracion del modulo, 
                 * que asegura que la frecuencia fue seteada de manera correcta
                 */  

  //Configuracion de las interrupciones
  attachInterrupt(digitalPinToInterrupt(2), Interrupt1, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), Interrupt2, FALLING);
  attachInterrupt(digitalPinToInterrupt(20), Interrupt3, FALLING);
}

void loop() {
  //Espera de datos provenientes del modulo
  if (Serial1.available()) {
    int DataIn = Serial1.read(); //Lectura del dato
    int ID = DataIn & 0xF0;      //Separacion de la direccion
    Serial.println("recibiendo..");
    Serial.print("ID: ");
    Serial.print(ID,HEX);      //Impresion del valor leido, solo para depuracion
    if (ID == my_addr)
    {
      Serial.println(" correcta ");
      Serial.print("dato: ");
      Serial.println(DataIn&0x0F,HEX);  
      //Separacion de los valores de cada LED 
      bool LED1_s = DataIn & 0x01;
      bool LED2_s = DataIn & 0x02;
      bool LED3_s = DataIn & 0x04;
      //Impresion de los valores recibidos
      digitalWrite(LED1, LED1_s);
      digitalWrite(LED2, LED2_s);
      digitalWrite(LED3, LED3_s);      
    }else Serial.println();
  }

 //Evaluacion de banderas de envio de datos
  if(send_dev1==true)
  {
    //Lectura de los valores de los switch
    uint8_t data = digitalRead(10)*4 +digitalRead(9)*2 + digitalRead(8);
    Serial1.write(addr1 + data); //Envio de los valores a los otros modulos, con la direccion correspondiente
     //Solo para depuracion 
    Serial.print("address: ");Serial.println(addr1,HEX);
    Serial.print("data: ");Serial.println(data,HEX);
    Serial.print("enviando a dev1 ");
    Serial.println(addr1 + data,HEX);
    
    send_dev1=false; //Reestablecimiento de la bandera a falso despues de haber completado el envio
  }else if(send_dev2==true)
  {
    //Lectura de los valores de los switch
    uint8_t data = digitalRead(10)*4 +digitalRead(9)*2 + digitalRead(8);
    Serial1.write(addr2 + data); //Envio de los valores a los otros modulos, con la direccion correspondiente
     //Solo para depuracion 
    Serial.print("address: ");Serial.println(addr2,HEX);
    Serial.print("data: ");Serial.println(data);
    Serial.print("enviando a dev2 ");
    Serial.println(addr2 + data,HEX);
    
    send_dev2=false; //Reestablecimiento de la bandera a falso despues de haber completado el envio
  }else if(send_dev3==true)
  {
    //Lectura de los valores de los switch
    uint8_t data = digitalRead(10)*4 +digitalRead(9)*2 + digitalRead(8);
    Serial1.write(addr3 + data); //Envio de los valores a los otros modulos, con la direccion correspondiente
     //Solo para depuracion 
    Serial.print("address: ");Serial.println(addr3,HEX);
    Serial.print("data: ");Serial.println(data); 
    Serial.print("enviando a dev3 ");
    Serial.println(addr3 + data,HEX);
    
    send_dev3=false; //Reestablecimiento de la bandera a falso despues de haber completado el envio
  }
  
}
//Declaracion de las rutinas de interrupcion
void Interrupt1() { 
  if ((long)(micros() - last_micros) >= debouncing_time * 1000) { //Espera de tiempo de estabilizacion del switch
    send_dev1=true; //Activacion de la bandera de envio al dispositivo1
    last_micros = micros();
  }
}

void Interrupt2() { 
  if ((long)(micros() - last_micros) >= debouncing_time * 1000) { //Espera de tiempo de estabilizacion del switch
    send_dev2=true; //Activacion de la bandera de envio al dispositivo2
    last_micros = micros();
  }
}

void Interrupt3() {
  if ((long)(micros() - last_micros) >= debouncing_time * 1000) { //Espera de tiempo de estabilizacion del switch
    send_dev3=true; //Activacion de la bandera de envio al dispositivo3
    last_micros = micros();
  }
}
//Funcion de configuracion del modulo APC220
void ConfigAPC() 
{
    // Procedimiento de configuración a 450MHz
    Serial.println("Configurando a 450MHz ...");
    digitalWrite(SetPin, LOW); //Escritura en bajo del pin SET
    delay(2); //Espera necesari antes de poder enviarle comandos
    Serial1.end();
    Serial1.begin(9600); //Cambio de velocidad del UART, a la velocidad de configuracion
    Serial1.println("WR 450000 4 9 6 0"); //Envio de datos de configuracion
    delay(202); //Espera antes de la respuesta por parte del modulo

    if (Serial1.available()) //Verificacion de que el modulo a contestado
    {
      String data = Serial1.readString();
      Serial.println(data);  //Solo para depuracion
    }
    digitalWrite(SetPin, HIGH); //Restablecimiento del pin al valor de alto
    delay(11); //Espera antes de que el modulo vuelva al modo de activo
    Serial1.end();
    Serial1.begin(57600); //Cambio de velocidad del UART a la velocidad de comunicacion maxima
    Serial.println("Configurado!!!"); //Solo para depuracion
}
