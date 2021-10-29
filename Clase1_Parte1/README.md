# RTO2
Autor: Fernando Prokopiuk <fernandoprokopiuk@gmail.com>

Parte 1:
Implemente un sistema que tenga las siguientes tareas y use los siguientes recursos:

* Recurso 1: Una cola "cola_1" cuyos elementos sean referencias de memoria (punteros)

* Tarea A: Encender periódicamente un LED. Cada vez que el LED encienda, se deberá enviar un mensaje "LED ON" a la "cola_1"

* Tarea B: Lectura con antirrebote de 2 teclas y medición de tiempo de pulsación. Al medirse el tiempo de pulsación, enviar el mensaje "TECx Tyyyy" a la "cola_1" donde x es el índice de tecla e yyyy la cantidad de ms que fue pulsada.

* Tarea C: Deberá obtener de "cola_1" mensajes de texto, y enviarlos por la uart (use printf)
