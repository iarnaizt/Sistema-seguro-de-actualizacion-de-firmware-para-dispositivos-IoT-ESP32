#ifndef INCLUDE_Nano_ESP32_OTA
#define INCLUDE_Nano_ESP32_OTA 1


#define DEBUG  1                                     // En modo producción se elimina esta definición

// -----------------------------------------------------------
//
// Pines de I/O
//
// -----------------------------------------------------------

#define PIN_SE_DHT22                 D2  
#define PIN_LED_VERDE                D5 
#define PIN_LED_ROJO                 D6  

// -----------------------------------------------------------
//
// Defines 
//
// -----------------------------------------------------------

#define ID_LED_ROJO                  1               // Identifica el Led ROJO
#define ID_LED_VERDE                 2               // Identifica el Led VERDE         

#define LED_ON                       0               // Led encendido
#define LED_OFF                      1               // Led apagado 
#define LED_BLINK                    2               // Led parpadeo (Blink)

#define PAUSA_INICIAL_ESP32          5000            // Pausa (ms) inicial al comienzo del setup del Dispositivo
#define PAUSA_GENERAL                1000            // Pausa (ms) generica
#define PAUSA_BLINK                  500             // Pausa (ms) para generar blink en los leds y pitofono
#define PAUSA_INI_DISPOSITIVO        5000            // Pausa (ms) para determinar el inicio TOTAL del Dispositivo
#define PAUSA_WATCHDOG_ESP32         35              // Pausa (s)  para modificar el tiempo de watchdog por defecto del ESP32
#define PAUSA_ACTUALIZAR_DHT22       30000           // Pausa (ms) para actualizar los datos del DHT22
#define PAUSA_ACTUALIZAR_OTA         5000            // Pausa (ms) para comprobar actualizaciones OTA
#define TIMEOUT_HTTP                 5000            // Utilizado en conexiones http
#define TIMEOUT_CNX_WIFI             10              // Utilizado en la conexion a WIFI (seg)

#define UART_VELOCIDAD_ESP32         115200          // Velocidad del puerto serie 

// ----------------------------------------------------
// Definiciones relacionadas con el control general del
// programa y que se  asignan  en  la  variable  global
// "estadoPrograma"
// ----------------------------------------------------

#define ESTADO_PROG_DEFECTO           0x00            // Este valor es para inicializar la variable estadoPrograma
#define ESTADO_PROG_INI               0x01            // Este valor indica que se acaba de inicializar el dispositivo
#define ESTADO_PROG_CNX               0x02            // Este valor indica que se esta en el proceso de conectando a WIFI/MQTT
#define ESTADO_PROG_OK                0x03            // Este valor indica que todo esta correcto, programa funcionando normalmente
#define ESTADO_PROG_STOP              0x04            // Este valor indica que se ha producido algun error y el programa esta parado
#define ESTADO_PROG_ERROR_WIFI        0x05            // Este valor indica error conectando a WIFI
#define ESTADO_PROG_ERROR_PARTICIONES 0x06            // Este valor indica error con las particiones



// ----------------------------------------------------
// 
// Defines relacionados con la gestion del WIFI y el 
// servidor de actualizaciones
//
// ----------------------------------------------------
                                                     
#define WIFI_CONECTADO               0               // Retornos de la funcion cnx_WIFI() 
#define WIFI_ERROR                   1
#define WIFI_CANCELADO               2

const char SERVIDOR_ACTUALIZACIONES_IP     [] = "192.168.1.45"; // Se DEBE indicar la IP del servidor, NO utilizar "localhost"
const char SERVIDOR_ACTUALIZACIONES_PUERTO [] = "8000";
const char SERVIDOR_CARPETA_FIRMWARE       [] = "firmware";
const char IDE_FICHERO_VERSION             [] = "version.txt";
const char IDE_FICHERO_FIRMWARE            [] = "firmware.bin";

const char IDE_VERSION_ACTUAL              [] = "1.0.0";
const char IDE_VERSION_MAX_CARACTERES         = 5;


// ----------------------------------------------------
//
// Mensajes y textos
//
// ----------------------------------------------------

const char MSG_IP                            [] = "Mi IP";

const char MSG_ESTADO_PROG_DEFECTO           [] = "PRG: Defecto";
const char MSG_ESTADO_PROG_INI               [] = "PRG: Inicio";
const char MSG_ESTADO_PROG_CNX               [] = "PRG: Conexion";
const char MSG_ESTADO_PROG_OK                [] = "PRG: Funcionamiento Normal";
const char MSG_ESTADO_PROG_STOP              [] = "PRG: Stop";
const char MSG_ESTADO_PROG_ERROR_WIFI        [] = "PRG: Error WIFI";
const char MSG_ESTADO_PROG_ERROR_PARTICIONES [] = "PRG: Error Particiones";

// -----------------------------------------------------------
// Definiciones relacionadas con sensores
// -----------------------------------------------------------

#define VALOR_SENSOR_ERROR ((float)-1)

// -----------------------------------------------------------
//
//             Definicion de TIPOS especiales
//
// -----------------------------------------------------------






#endif
