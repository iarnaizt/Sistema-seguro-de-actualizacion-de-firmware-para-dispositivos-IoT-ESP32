
#include <WiFi.h>
#include "Nano_ESP32_OTA.h"


// ----------------------------------------------------
//
// int cnx_WIFI(void)
// Retorna:
//  WIFI_CONECTADO 
//  WIFI_ERROR     
//  WIFI_CANCELADO 
//
// ----------------------------------------------------
int cnx_WIFI(void)
{
  int    resultado;
  int    timeoutCnx;
    


  resultado  = WIFI_ERROR;
  timeoutCnx = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin("MOVISTAR_2707","xx");
        
  while ( true )
        { 
          esp_task_wdt_reset(); 
                 
          if ( WiFi.status()==WL_CONNECTED )
             {
               resultado = WIFI_CONECTADO;
               #ifdef DEBUG
               Serial.println("Conectado...");
               #endif
               break;
             }
          else
             {
               espera(PAUSA_GENERAL);
               timeoutCnx++;
               if ( timeoutCnx>TIMEOUT_CNX_WIFI )
                  {               
                    resultado  = WIFI_ERROR;                     
                    #ifdef DEBUG
                    Serial.println("Error conexion...");
                    #endif
                    break;
                  }
             }     
        }
 
  return( resultado );
}


