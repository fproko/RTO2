/*=============================================================================
 * Fernando Prokopiuk <fernandoprokopiuk@gmail.com>
 * Date: 2021/10/29
 * Version: v1.0
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/
#include "keys.h"

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "queue.h"

#include "string.h"
/*=====[ Definitions of private data types ]===================================*/
const t_key_config  keys_config[] = { TEC1, TEC2 };

#define KEY_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
/*=====[Definition macros of private constants]==============================*/
#define DEBOUNCE_TIME   40
#define DEBOUNCE_TIME_MS pdMS_TO_TICKS(DEBOUNCE_TIME)
#define MSG_KEY_SIZE 10
/*=====[Prototypes (declarations) of private functions]======================*/

static void keys_ButtonError( uint32_t index );
static void buttonPressed( uint32_t index );
static void buttonReleased( uint32_t index );

/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of public global variables]==============================*/
t_key_data keys_data[KEY_COUNT];
extern QueueHandle_t queue_print;
/*=====[prototype of private functions]=================================*/
void task_B( void* taskParmPtr );

/*=====[Implementations of public functions]=================================*/
TickType_t get_diff( uint32_t index )
{
    TickType_t tiempo;

    taskENTER_CRITICAL();
    tiempo = keys_data[index].time_diff;
    taskEXIT_CRITICAL();

    return tiempo;
}

void clear_diff( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].time_diff = KEYS_INVALID_TIME;
    taskEXIT_CRITICAL();
}

void keys_Init( void )
{
    BaseType_t res;
    uint32_t i;

    for ( i = 0 ; i < KEY_COUNT ; i++ )
    {
        keys_data[i].state          = BUTTON_UP;  // Set initial state
        keys_data[i].time_down      = KEYS_INVALID_TIME;
        keys_data[i].time_up        = KEYS_INVALID_TIME;
        keys_data[i].time_diff      = KEYS_INVALID_TIME;
    }
    // Crear tareas en freeRTOS
    res = xTaskCreate (
              task_B,					// Funcion de la tarea a ejecutar
              ( const char * )"task_B",	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
              0,							// Parametros de tarea
              tskIDLE_PRIORITY+1,			// Prioridad de la tarea
              0							// Puntero a la tarea creada en el sistema
            );

    // Gestión de errores
    configASSERT( res == pdPASS );
}

// keys_ Update State Function
void keys_Update( uint32_t index )
{
    switch( keys_data[index].state )
    {
        case STATE_BUTTON_UP:
            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_FALLING;
            }
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_DOWN;

                /* ACCION DEL EVENTO !*/
                buttonPressed( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_UP;
            }

            /* LEAVE */
            break;

        case STATE_BUTTON_DOWN:
            /* CHECK TRANSITION CONDITIONS */
            if( gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_RISING;
            }
            break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_UP;

                /* ACCION DEL EVENTO ! */
                buttonReleased( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_DOWN;
            }

            /* LEAVE */
            break;

        default:
            keys_ButtonError( index );
            break;
    }
}

/*=====[Implementations of private functions]================================*/

/* accion de el evento de tecla pulsada */
static void buttonPressed( uint32_t index )
{
    TickType_t current_tick_count = xTaskGetTickCount();

    taskENTER_CRITICAL();
    keys_data[index].time_down = current_tick_count;
    taskEXIT_CRITICAL();
}

/* accion de el evento de tecla liberada */
static void buttonReleased( uint32_t index )
{
    TickType_t current_tick_count = xTaskGetTickCount();

    taskENTER_CRITICAL();
    keys_data[index].time_up    = current_tick_count;
    keys_data[index].time_diff  = keys_data[index].time_up - keys_data[index].time_down;
    taskEXIT_CRITICAL();

    if ( keys_data[index].time_diff > 0 )
    {
        char msg_key[MSG_KEY_SIZE];
        sprintf(msg_key, "TEC%1d T%04d", index+1, keys_data[index].time_diff);

        taskENTER_CRITICAL();   
        char *pKEY = pvPortMalloc(sizeof(char)*MSG_KEY_SIZE);
        taskEXIT_CRITICAL();

        if (pKEY != NULL)
        {
            memcpy(pKEY, msg_key, MSG_KEY_SIZE);
            xQueueSend(queue_print, &pKEY, portMAX_DELAY);
        }
        else
        {
            printf(MALLOC_FAILED);
        }
    }
}

static void keys_ButtonError( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].state = BUTTON_UP;
    taskEXIT_CRITICAL();
}

/*=====[Implementations of private functions]=================================*/
void task_B( void* taskParmPtr )
{
    uint32_t i;
    while( TRUE )
    {
        for ( i = 0 ; i < KEY_COUNT ; i++ )
        {
            keys_Update( i );
        }
        vTaskDelay( DEBOUNCE_TIME_MS );
    }
}
