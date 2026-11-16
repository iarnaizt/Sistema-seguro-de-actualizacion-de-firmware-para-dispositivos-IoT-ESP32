//! ----------------------------------------------------------
//!
//!           
//!          NOTA (1):
//!           Para subir el programa al Arduino Nano ESP32 hacer:
//!             Con la alimentacion puesta, pulsar rapidamente el pulsador de RESET
//!             hasta que el led tricolor del Arduino se quede fijo, normalmente en
//!             color verde, entonces en el menu de herramientas poner las opciones 
//!             de la NOTA (2)
//!   
//!          NOTA (2):
//!           En el IDE de Arduino utilizar (DEBEN seleccionarse estas):
//!             Herramientas -> Placa:            Arduino Nano ESP32          
//!             Herramientas -> Puerto:           dfu puertos
//!             Herramientas -> Partition Scheme: With FAT partition (default)
//!             Herramientas -> USB Mode:         Debug mode (Hardware CDC)
//!             Herramientas -> Programador:      Esptool
//!             Subir ya el programa como se hace normalmente
//!
//! Versión de SW 1.0
//! Versión de IDE Arduino: 2.3.8   
//!          
//! ----------------------------------------------------------


#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <DHT.h>
#include <esp_ota_ops.h>
#include "Nano_ESP32_OTA.h"



// -----------------------------------------------------------
//
// Variables Globales
//
// -----------------------------------------------------------

byte                   estadoPrograma;                              // Variable que contiene el estado del programa
byte                   modoLedRojo;                                 // Modo en el que se encuentra el led ROJO
byte                   modoLedVerde;                                // Modo en el que se encuentra el led VERDE 
byte                   flgReintentarCnx;                            // Flag para indicar si se esta intentado reconectar
byte                   mBlink;                                      // Variable asociada al parpadeo de los Leds
unsigned long int      timerBlink;                                  // Timer para generar el parpadeo de los Leds    
unsigned long int      timerDHT22;                                  // Timer para comprobar el sensor DHT22
unsigned long int      timerOTA;                                    // Timer para comprobar si hay actualizaciones OTA

DHT sensorTH(PIN_SE_DHT22, DHT22);                                  // Sensor de Temperatura/Humedad DHT22

const esp_partition_t* particionActiva;                             // Datos de la particion ACTIVA      
const esp_partition_t* particionAlternativa;                        // Datos de la particion Alternativa
                                                                    // Los campos de una estructura esp_partition_t son:
                                                                    //
                                                                    //  type (esp_partition_type_t):
                                                                    //  Es un tipo enumerado que representa la categoría general de la partición
                                                                    //  (si es de aplicación o de datos).
                                                                    //
                                                                    //  subtype (esp_partition_subtype_t):
                                                                    //  Es un tipo enumerado que indica el rol específico de la partición (por ejemplo, 
                                                                    //  si es la de fábrica, app_0, app_1, NVS, SPIFFS, etc.).
                                                                    //
                                                                    //  address (uint32_t):
                                                                    //  Es un entero sin signo de 32 bits. Guarda la dirección física de inicio
                                                                    //  de la partición en la memoria flash.
                                                                    //
                                                                    //  size (uint32_t):
                                                                    //  Es un entero sin signo de 32 bits. Indica el tamaño total de la partición en bytes.
                                                                    //
                                                                    //  label (char[17]):
                                                                    //  String de caracteres con un tamaño fijo de 17 bytes.
                                                                    //  Almacena el nombre asignado a la partición
                                                                    //
                                                                    //  encrypted (bool):
                                                                    //  Es un valor booleano que indica si la partición está protegida mediante el 
                                                                    //  cifrado de flash (flash encryption) del ESP32.

// -----------------------------------------------------------
//
// setup()
//
// -----------------------------------------------------------
void setup()
{
  
  espera(PAUSA_INICIAL_ESP32);                       // Pausa inicial NECESARIA para esperar a que el ESP32
                                                     // se estabilice en el arranque
  activar_WDT();                                     // Reactivar Watchdog
  
  pinMode(PIN_LED_ROJO   ,OUTPUT);
  pinMode(PIN_LED_VERDE  ,OUTPUT);
    
  Serial.begin(UART_VELOCIDAD_ESP32);
  sensorTH.begin();

  estadoPrograma = ESTADO_PROG_INI;

  modoLedRojo  = LED_ON;
  modoLedVerde = LED_OFF;
  mBlink       = false;
  timerBlink   = millis();

  obtenerInfoParticiones(); 
}



// -----------------------------------------------------------
//
// loop 
//
// -----------------------------------------------------------
void loop()
{
   esp_task_wdt_reset(); 

   set_Blink();
   actualizar_Leds();
      
   // --------------------------------------------------------
   //
   //
   // --------------------------------------------------------
   switch( estadoPrograma )
         {
           case ( ESTADO_PROG_INI ):
                { // --------------------------------------------------------
                  //
                  // Inicializacion del Dispositivo
                  //
                  // --------------------------------------------------------
                 
                  flgReintentarCnx  = false;
                                    
                  // --------------------------------------------------------
                  //
                  //   Aqui inicializaciones propias de la aplicacion
                  //                          .
                  //                          .
                  //                          .
                  // --------------------------------------------------------
                  if ( (particionActiva==NULL) || (particionAlternativa==NULL) )
                     {
                       cambiarEstadoPrograma(ESTADO_PROG_ERROR_PARTICIONES);
                     }
                  else
                     {
                       cambiarEstadoPrograma(ESTADO_PROG_CNX);
                     } 
                  
                  break;
                } 

           case ( ESTADO_PROG_CNX ):
                { // --------------------------------------------------------
                  //
                  // Conexion a WIFI 
                  //
                  // --------------------------------------------------------
                  
                  switch( cnx_WIFI() )
                        {
                          case ( WIFI_CONECTADO ):
                               { // ------------------------------------------
                                 // Conectado a una WIFI
                                 // ------------------------------------------

                                 #ifdef DEBUG
                                 Serial.print(MSG_IP);
                                 Serial.print(": ");
                                 Serial.println(WiFi.localIP());
                                 #endif
                                 
                                 espera(PAUSA_GENERAL*3);
                                                          
                                 modoLedRojo         = LED_OFF;
                                 modoLedVerde        = LED_ON;
                                 flgReintentarCnx    = false;
                                 timerDHT22          = millis();
                                 timerOTA            = millis();

                                 cambiarEstadoPrograma(ESTADO_PROG_OK);
                                 break;
                               }

                          case ( WIFI_ERROR ):
                               { // ------------------------------------------
                                 // NO se puede conectar a una WIFI
                                 // ------------------------------------------                                 
                                 cambiarEstadoPrograma(ESTADO_PROG_ERROR_WIFI);       
                                 break;
                               }
                               
                          case ( WIFI_CANCELADO ):
                               { // ------------------------------------------
                                 // Cancelado el intento de conexion WIFI
                                 // ------------------------------------------
                                 cambiarEstadoPrograma(ESTADO_PROG_INI);     
                                 break;
                               }                               
                        }
                  
                  break;
                } 

           case ( ESTADO_PROG_OK ):
                { // --------------------------------------------------------
                  //
                  // Estado NORMAL de funcionamiento del Dispositivo
                  //
                  // --------------------------------------------------------

                  if ( WiFi.status()==WL_CONNECTED )
                     {
                       actualizar_DHT22();
                       comprobar_OTA();
                     }
                  else
                     {
                       cambiarEstadoPrograma(ESTADO_PROG_ERROR_WIFI);  
                     }   

                  break;
                }            

           case ( ESTADO_PROG_ERROR_WIFI ):
                { // --------------------------------------------------------
                  //
                  // Error conectando a WIFI
                  // 
                  // --------------------------------------------------------
                  modoLedRojo      = LED_ON;
                  flgReintentarCnx = true;
                  espera(PAUSA_GENERAL*3);
                  cambiarEstadoPrograma(ESTADO_PROG_CNX);
                  break;
                } 
           
           case ( ESTADO_PROG_ERROR_PARTICIONES ):
                { // --------------------------------------------------------
                  //
                  // Error en las particiones PARTICIONES
                  // 
                  // --------------------------------------------------------
                  modoLedRojo = LED_BLINK;
      
      
      
                  break;
                } 

         }

}



// -----------------------------------------------------------
//
// void set_Blink(void) 
//
// -----------------------------------------------------------
void set_Blink(void)
{
  if ( (millis()-timerBlink)>PAUSA_BLINK )
     {
       timerBlink = millis(); 
       if ( mBlink==false ) { mBlink = true;  }
       else                 { mBlink = false; }
     }
}



// ----------------------------------------------------
//
// void espera(unsigned int t_delay)
//
// ----------------------------------------------------
void espera(unsigned int t_delay)
{
  unsigned long int vPausa;     
  
  vPausa = millis();

  while ( (millis()-vPausa)<t_delay )
        {
          
          esp_task_wdt_reset();

        } 
}



// ----------------------------------------------------
//
// void actualizar_Leds(void)
//
// ----------------------------------------------------
void actualizar_Leds(void)
{
  switch( modoLedRojo )
        {
          case ( LED_ON ):
               { 
                 digitalWrite(PIN_LED_ROJO,HIGH);
                 break;
               }  
          case ( LED_OFF ):
               {
                 digitalWrite(PIN_LED_ROJO,LOW);
                 break;
               }  
          case ( LED_BLINK ):
               { 
                 if ( mBlink==true ) { digitalWrite(PIN_LED_ROJO,HIGH); }
                 else                { digitalWrite(PIN_LED_ROJO,LOW);  } 
                 break;
               }
        }
  
  switch( modoLedVerde )
        {
          case ( LED_ON ):
               { 
                 digitalWrite(PIN_LED_VERDE,HIGH);
                 break;
               }  
          case ( LED_OFF ):
               {
                 digitalWrite(PIN_LED_VERDE,LOW);
                 break;
               }  
          case ( LED_BLINK ):
               { 
                 if ( mBlink==true ) { digitalWrite(PIN_LED_VERDE,HIGH); }
                 else                { digitalWrite(PIN_LED_VERDE,LOW);  } 
                 break;
               }
        }
}



// ----------------------------------------------------
//
// void cambiarEstadoPrograma(int estadoID )
//
// ----------------------------------------------------
void cambiarEstadoPrograma(int estadoID )
{
  if ( estadoPrograma!=estadoID )
     {
       estadoPrograma = estadoID;

       #ifdef DEBUG
       Serial.println(getEstadoPrograma());
       #endif     
     }
}



// ----------------------------------------------------
//
// const char* getEstadoPrograma(void)
//
// ----------------------------------------------------
const char* getEstadoPrograma(void)
{
  const char* estadoP;

  estadoP = "";

  switch(estadoPrograma) 
        {
          case ( ESTADO_PROG_DEFECTO           ): { estadoP = MSG_ESTADO_PROG_DEFECTO;           break; }
          case ( ESTADO_PROG_INI               ): { estadoP = MSG_ESTADO_PROG_INI;               break; }   
          case ( ESTADO_PROG_CNX               ): { estadoP = MSG_ESTADO_PROG_CNX;               break; }
          case ( ESTADO_PROG_OK                ): { estadoP = MSG_ESTADO_PROG_OK;                break; }
          case ( ESTADO_PROG_STOP              ): { estadoP = MSG_ESTADO_PROG_STOP;              break; }
          case ( ESTADO_PROG_ERROR_WIFI        ): { estadoP = MSG_ESTADO_PROG_ERROR_WIFI;        break; }
          case ( ESTADO_PROG_ERROR_PARTICIONES ): { estadoP = MSG_ESTADO_PROG_ERROR_PARTICIONES; break; }
        }
  
  return( estadoP );
}



// ----------------------------------------------------
//
// void actualizar_DHT22(void)
//
// ----------------------------------------------------
void actualizar_DHT22(void)
{
  float vTE;
  float vHU;
  float vHI;

  if ( (millis()-timerDHT22)>PAUSA_ACTUALIZAR_DHT22 )
     { // ----------------------------------------------------
       //
       // ----------------------------------------------------
       
       vTE = sensorTH.readTemperature();
       vHU = sensorTH.readHumidity();

       if ( isnan(vTE) ) { vTE = VALOR_SENSOR_ERROR; }
       if ( isnan(vHU) ) { vHU = VALOR_SENSOR_ERROR; }
       if ( vTE!=VALOR_SENSOR_ERROR && vHU!=VALOR_SENSOR_ERROR ) { vHI = sensorTH.computeHeatIndex(vTE, vHU, false); }
       else                                                      { vHI = VALOR_SENSOR_ERROR;                         }

       Serial.printf("\nTemperatura: %.1fC", vTE);
       Serial.printf("\nHumedad: %.1f%%",vHU);          
       Serial.printf("\nS. Termica: %.1fC",vHI);          

       timerDHT22 = millis();
     }
}



// ----------------------------------------------------
//
// void comprobar_OTA(void)
//
// ----------------------------------------------------
void comprobar_OTA(void)
{
  String     urlVersion;
  String     versionServidor;
  String     versionActual;
  WiFiClient cliente;
  HTTPClient http;
  int        resultado;

  if ( (millis()-timerOTA)>PAUSA_ACTUALIZAR_OTA )
     { // ----------------------------------------------------
       // Temporizado, comprueba  si  hay una nueva version de
       // Firmware
       // ----------------------------------------------------
       
       urlVersion  = String("http://"); 
       urlVersion += String(SERVIDOR_ACTUALIZACIONES_IP);
       urlVersion += String(":");
       urlVersion += String(SERVIDOR_ACTUALIZACIONES_PUERTO);
       urlVersion += String("/");
       urlVersion += String(SERVIDOR_CARPETA_FIRMWARE);
       urlVersion += String("/");
       urlVersion += String(IDE_FICHERO_VERSION);
       
       resultado = testFichero(urlVersion);

       if ( resultado==true )
          { // -----------------------------------------
            // Fichero accesible en el Servidor
            // -----------------------------------------

            resultado = false;

            #ifdef DEBUG
            Serial.printf("\nOTA: Accediendo a: %s",urlVersion.c_str());
            #endif

            http.setTimeout(TIMEOUT_HTTP);
            if ( http.begin(cliente, urlVersion)==true )
               {   
                 if ( http.GET()==HTTP_CODE_OK )
                    {
                      resultado = true;
                    }
               }

            if ( resultado==true )
               { // ----------------------------------------------------
                 // Recupera version  publicada  y si procede realiza la
                 // actualizacion
                 // ----------------------------------------------------
                 versionServidor = recuperaVersionPublicada(&http);
                 versionActual   = IDE_VERSION_ACTUAL;
                 http.end();

                 #ifdef DEBUG
                 Serial.printf("\nOTA: Version instalada=%s", versionActual  );
                 Serial.printf("\nOTA: Version publicada=%s", versionServidor);
                 #endif

                 switch( compararVersiones(versionActual,versionServidor) )
                       {
                         case( 0 ):
                             { // ------------------------------------------
                               // Versiones Iguales
                               // ------------------------------------------
                          
                               break;
                             } 
                         case( 1 ):
                             { // ------------------------------------------
                               // Version en el servidor MENOR que la actual
                               // ------------------------------------------
                          
                               break;
                             } 
                         case( 2 ):
                             { // ------------------------------------------
                               // Version en el servidor MAYOR que la actual
                               // ------------------------------------------
                               actualizarFirmware();
                               break;
                             } 
                        }
                 }
              else
                 {
                   http.end();
                 }
          }
       
       if ( resultado==false )   
          {
            #ifdef DEBUG
            Serial.printf("\nOTA: ERROR accediendo a: %s",urlVersion.c_str());
            #endif
          }

       timerOTA = millis();
     }

}



// ----------------------------------------------------
//
// void obtenerInfoParticiones(void)
//
// particionActiva:    Datos de partición activa
// particionSiguiente: Datos de partición Alternativa
// ----------------------------------------------------
void obtenerInfoParticiones(void)
{

   particionActiva      = esp_ota_get_running_partition();
   particionAlternativa = esp_ota_get_next_update_partition(NULL);

   #ifdef DEBUG
   // ----------------------------------------------------
   // Info de la particion ACTIVA
   // ----------------------------------------------------
   Serial.printf("\n");
   Serial.printf("------------------------------\n");
   if ( particionActiva!=NULL )
      {
        Serial.printf("Particion activa: %s\n", particionActiva->label);
        Serial.printf("Direccion en Flash: 0x%08X\n", (uint32_t)particionActiva->address);
        Serial.printf("Particion: %lu Bytes\n"      , (uint32_t)particionActiva->size);
        if ( particionActiva->encrypted==true ) { Serial.printf("Particion Encriptada: SI\n"); }
        else                                    { Serial.printf("Particion Encriptada: NO\n"); }
      }
   else
      {
        Serial.printf("No se pudo obtener la partición en ejecución.");
      }
   Serial.printf("------------------------------\n");

   // ----------------------------------------------------
   // Info de la particion DESTINO ALTERNATIVO
   // ----------------------------------------------------
   Serial.printf("\n");
   Serial.printf("------------------------------\n");
   if ( particionAlternativa!=NULL )
      {
        Serial.printf("\n");
        Serial.printf("Particion alternativa: %s\n", particionAlternativa->label);
        Serial.printf("Direccion en Flash: 0x%08X\n", (uint32_t)particionAlternativa->address);
        Serial.printf("Particion: %lu Bytes\n"      , (uint32_t)particionAlternativa->size);
        if ( particionAlternativa->encrypted==true ) { Serial.printf("Particion Encriptada: SI\n"); }
        else                                         { Serial.printf("Particion Encriptada: NO\n"); }
      }
   else
      {
        Serial.printf("No se pudo obtener la partición alternativa.");
      }
   Serial.printf("------------------------------\n");

   // ----------------------------------------------------
   // Info de la version actual
   // ----------------------------------------------------
   Serial.printf("\nNano ESP32, Version %s\n", IDE_VERSION_ACTUAL);

   #endif


}



// ----------------------------------------------------
//
// String recuperaVersionPublicada(HTTPClient* ptrHttp)
//
// ----------------------------------------------------
static String strVersion;

String recuperaVersionPublicada(HTTPClient* ptrHttp)
{
  WiFiClient* stream;
  char        c;

  stream     = ptrHttp->getStreamPtr();
  strVersion = "";
    
  while (ptrHttp->connected() && stream->available() && (strVersion.length()<IDE_VERSION_MAX_CARACTERES) )
        { 
          c = stream->read();
          strVersion += c;
        }
  strVersion.trim();

  return( strVersion );
}



// ----------------------------------------------------
//
// int actualizarFirmware(void)
//
// ----------------------------------------------------
int actualizarFirmware(void)
{
  String     urlFirmware;
  WiFiClient descarga;
  int        resultado;

  urlFirmware  = String("http://"); 
  urlFirmware += String(SERVIDOR_ACTUALIZACIONES_IP);
  urlFirmware += String(":");
  urlFirmware += String(SERVIDOR_ACTUALIZACIONES_PUERTO);
  urlFirmware += String("/");
  urlFirmware += String(SERVIDOR_CARPETA_FIRMWARE);
  urlFirmware += String("/");
  urlFirmware += String(IDE_FICHERO_FIRMWARE);

  resultado = testFichero(urlFirmware);
  
  if ( resultado==true )
     { // -----------------------------------------
       // Fichero accesible en el Servidor
       // -----------------------------------------
       
       httpUpdate.rebootOnUpdate(true);                                          // Reinicia si la actualizacion se completa con éxito

       // -----------------------------------------------------------------------------  
       // Define una función especial de callback usando una función lambda []()
       // Esta línea le dice al programa: "Justo en el // instante en que te conectes
       // con éxito al servidor y vayas a empezar a escribir el primer byte en la
       // memoria flash, ejecuta este código En este caso, imprime un aviso en el
       // monitor serie para indicar que el proceso comenzó.
       // -----------------------------------------------------------------------------
       httpUpdate.onStart([]() { Serial.println("OTA: descarga iniciada"); });   

       // -----------------------------------------------------------------------------
       // Configura una función de callback que la librería invocará repetidamente
       // cada vez que reciba un paquete de bytes desde el servidor.
       // Recibe dos variables automáticas de la librería:
       // actual (bytes descargados hasta el momento)
       // total  (tamaño total en bytes del archivo binario que se extrae de las cabeceras HTTP).
       // La fórmula calcula el porcentaje de avance multiplicando de forma segura
       // (con 100LL para evitar desbordamientos de enteros) y divide entre el total.
       // Envia por el puerto serie salidas tipo: OTA: 15 %, OTA: 16 %, etc.
       // -----------------------------------------------------------------------------
       httpUpdate.onProgress([](int actual, int total) { if (total > 0) Serial.printf("OTA: %d %%\n", (int)(100LL * actual / total)); } );

       // -----------------------------------------------------------------------------
       // . esp_task_wdt_deinit();
       //   Detiene el Watchdog
       //
       // . httpUpdate.update(descarga, urlFirmware);
       //   Detiene momentáneamente el flujo normal del programa, abre la conexión con
       //   el servidor usando el cliente descarga, solicita el archivo de la
       //   urlFirmware, empieza a recibir los datos en bloques y los escribe
       //   directamente en la partición OTA alternativa de la memoria flash del ESP32
       //   El proceso es síncrono (bloqueante) y al terminar guarda el resultado
       //   final en la variable resultado.
       //
       // . activar_WDT();
       //   Vuelve a activar el Watchdog
       // -----------------------------------------------------------------------------
       esp_task_wdt_deinit(); 
       t_httpUpdate_return operacion = httpUpdate.update(descarga, urlFirmware);
       activar_WDT();

       switch( operacion )
             {
               case( HTTP_UPDATE_FAILED ):
                   { // -------------------------------------------
                     // Error, se puede ver con
                     // httpUpdate.getLastError()
                     // httpUpdate.getLastErrorString().c_str()
                     // -------------------------------------------
                     resultado = false;
                     break;
                   }
               case( HTTP_UPDATE_NO_UPDATES ):
                   { // ------------------------------------------
                     // El servidor indica que no hay nueva imagen
                     // ------------------------------------------
                     resultado = false;
                     break;
                   }
               default:
                   { // ------------------------------------------
                     // OK
                     // ------------------------------------------
                     resultado = true;
                     break;
                   }
             } 
     }

  if ( resultado==false )   
     {
       #ifdef DEBUG
       Serial.printf("\nOTA: ERROR accediendo a: %s",urlFirmware.c_str());
       #endif
     }
  else
     {
       #ifdef DEBUG
       Serial.printf("\nOTA: Actualizacion Finalizada, Reiniciando...");
       #endif
     }

  return( resultado );
}


// ----------------------------------------------------
//
// int testFichero(String urlFichero)
// Retorna:
//  true  El fichero SI existe en el servidor
//  false El fichero NO existe en el servidor
//
// ----------------------------------------------------
int testFichero(String urlFichero)
{
  WiFiClient clienteVerificacion;
  HTTPClient http;
  bool       resultado;
  int        codigoHTTP;

  resultado = false;

  http.setTimeout(3000); // Un timeout corto es suficiente
  
  if ( http.begin(clienteVerificacion, urlFichero)==true )
     { // ----------------------------------------------------
       // Se utiliza http.sendRequest("HEAD") en lugar de
       // http.GET()
       // ----------------------------------------------------
       codigoHTTP = http.sendRequest("HEAD");

       switch( codigoHTTP )
             {
               case( HTTP_CODE_OK ):
                   { // ------------------------------------------------
                     // El archivo existe en el servidor (Código 200)
                     // ------------------------------------------------
                     resultado = true;
                     break;
                   }                   
               case( 404 ):
                   { // ------------------------------------------------
                     // El archivo no existe en el servidor (Código 404)
                     // ------------------------------------------------
                     break;
                   }
               default:
                   { // ------------------------------------------------
                     // El Servidor respondió con código inesperado
                     // ------------------------------------------------
                     break;
                   }    
             }
       http.end();
     }
  else
     { // ---------------------------------------------
       // No se pudo conectar para verificar el archivo
       // ---------------------------------------------
     }
  
  return resultado;
}



// ----------------------------------------------------
//
// int compararVersiones(String A, String B)
// Compara dos Strings que representan versiones en  el 
// formato X.X.X
// Retorna:
//  0 Las dos versiones son iguales
//  1 La version del String A es mayor que la del B
//  2 La version del String B es mayor que la del A
// ----------------------------------------------------
int compararVersiones(String A, String B)
{
  // Eliminar espacios y saltos de línea
  A.trim();
  B.trim();
  
  // -----------------------------------------
  // Si las cadenas son idénticas tras el trim
  // retorna 0
  // -----------------------------------------
  if ( A==B )
     {
       return 0;
     }

  // -----------------------------------------
  // Variables para almacenar los tres
  // componentes de cada versión
  // -----------------------------------------
  int majorA = 0, minorA = 0, patchA = 0;
  int majorB = 0, minorB = 0, patchB = 0;
  int primerPunto;
  int segundoPunto;

  // -----------------------------------------
  // Extraer los números de la versión A
  // -----------------------------------------
  primerPunto  = A.indexOf('.');
  segundoPunto = A.indexOf('.', primerPunto + 1);
  
  majorA = A.substring(0, primerPunto).toInt();
  minorA = A.substring(primerPunto + 1, segundoPunto).toInt();
  patchA = A.substring(segundoPunto + 1).toInt();

  // -----------------------------------------
  // Extraer los números de la versión B
  // -----------------------------------------
  primerPunto  = B.indexOf('.');
  segundoPunto = B.indexOf('.', primerPunto + 1);
  
  majorB = B.substring(0, primerPunto).toInt();
  minorB = B.substring(primerPunto + 1, segundoPunto).toInt();
  patchB = B.substring(segundoPunto + 1).toInt();

  // -----------------------------------------
  // Comparación de izquierda a derecha
  // Comparar dígito Mayor
  // -----------------------------------------
  if (majorA > majorB) return 1;
  if (majorA < majorB) return 2;

  // -----------------------------------------
  // Si los Mayores son iguales
  // comparar dígito Menor
  // -----------------------------------------
  if (minorA > minorB) return 1;
  if (minorA < minorB) return 2;

  // -----------------------------------------
  // Si los Menores son iguales
  // comparar Parche
  // -----------------------------------------
  if (patchA > patchB) return 1;
  if (patchA < patchB) return 2;

  // -----------------------------------------
  // Por si acaso algún formato fuera extraño
  // pero equivalente numéricamente (ej: 1.0 y 1.0.0)
  // -----------------------------------------
  return 0; 
}



// ----------------------------------------------------
// void activar_WDT(void)
// Reactivar el Wacthdog detectando la versión real del
// ESP32 en el IDE
// ----------------------------------------------------
void activar_WDT(void)
{
  
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  // --------------------------------------------------
  // El entorno usa ESP32 Core v3.0 o superior
  // (Usa la estructura moderna)
  // --------------------------------------------------
  esp_task_wdt_config_t twdt_config =
  {
    .timeout_ms     = PAUSA_WATCHDOG_ESP32 * 1000  ,  // Segundos a milisegundos
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,  // Vigila todos los núcleos
    .trigger_panic  = true                            // Reinicia si se congela
  };
  esp_task_wdt_init(&twdt_config); 
  #else
  // --------------------------------------------------
  // El entorno usa ESP32 Core v2.0.x o anterior
  // El primer parámetro es en SEGUNDOS, el segundo
  // activa el reinicio automático (panic)
  // --------------------------------------------------
  esp_task_wdt_init(PAUSA_WATCHDOG_ESP32, true); 
  #endif

  esp_task_wdt_add(NULL); 

}









