/*=============================================================================
 * Fernando Prokopiuk <fernandoprokopiuk@gmail.com>
 * Date: 2021/11/01
 * Version: v1.1
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "FreeRTOSConfig.h"
#include "keys.h"
#include "queue.h"

#include "string.h"

/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)
#define WELCOME_MSG  "RTO2 Clase 1 Parte 1.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error al crear la cola.\r\n"
#define LED_ERROR LEDR
#define N_QUEUE 	10

#define MSG_LED         "LED ON"
#define MSG_LED_SIZE    sizeof(MSG_LED)

#define MALLOC_FAILED "Malloc Failed\n"
/*==================[definiciones de datos internos]=========================*/
const gpioMap_t leds_t[] = {LEDB};
const gpioMap_t gpio_t[] = {GPIO7};
QueueHandle_t queue_print;
/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;

#define LED_COUNT   sizeof(leds_t)/sizeof(leds_t[0])
/*==================[declaraciones de funciones internas]====================*/
void gpio_init( void );
/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_A( void* taskParmPtr );
void tarea_C( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();

    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages
    printf( WELCOME_MSG );

    BaseType_t res;
    uint32_t i;

    // Crear tarea en freeRTOS
    res = xTaskCreate(
              tarea_A,                     // Funcion de la tarea a ejecutar
              ( const char * )"tarea_A",   // Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
              0,                          // Parametros de tarea
              tskIDLE_PRIORITY+1,         // Prioridad de la tarea
              0                           // Puntero a la tarea creada en el sistema
            );

    // Gestion de errores
    configASSERT( res == pdPASS );

    // Creo tarea unica de impresion
    res = xTaskCreate(
              tarea_C,                     // Funcion de la tarea a ejecutar
              ( const char * )"tarea_C",   // Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
              0,                          // Parametros de tarea
              tskIDLE_PRIORITY+1,         // Prioridad de la tarea
              0                           // Puntero a la tarea creada en el sistema
            );

    // Gestion de errores
    configASSERT( res == pdPASS );

    /* inicializo driver de teclas */
    keys_Init();

    // Crear cola
    queue_print = xQueueCreate(N_QUEUE, sizeof(void *));         //Cola de estructuras a imprimir.

    // Gestion de errores de colas
    configASSERT( queue_print != NULL );

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    // ---------- REPETIR POR SIEMPRE --------------------------
    configASSERT( 0 );

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return TRUE;
}

/*==================[definiciones de funciones internas]=====================*/
void gpio_init( void )
{
    uint32_t i;

    for( i = 0 ; i < LED_COUNT; i++ )
    {
        gpioInit ( gpio_t[i], GPIO_OUTPUT );
    }
}
/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_A( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char *pLED = NULL;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        gpioWrite( LEDB, ON );
        gpioWrite( GPIO7, ON );

        pLED = pvPortMalloc(MSG_LED_SIZE);

        if (pLED != NULL)
        {
            memcpy(pLED, MSG_LED, MSG_LED_SIZE);
            xQueueSend(queue_print, &pLED, portMAX_DELAY); //Se utiliza & para pasar una copia de la dirección.
        }
        else
        {
            printf(MALLOC_FAILED);
        }
        vTaskDelay(xPeriodicity/2);

        gpioWrite( LEDB, OFF );
        gpioWrite( GPIO7, OFF );
        vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
    }
}

void tarea_C( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
    char *msg = NULL;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        xQueueReceive(queue_print, &msg, portMAX_DELAY); // Esperamos dato para imprimir
        printf("%s\r\n", msg);

        vPortFree(msg); //Libero memoria

        msg = NULL;
    }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( "Malloc Failed Hook!\n" );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
