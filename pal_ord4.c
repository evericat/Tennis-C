#include <pthread.h>
#include "winsuport2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "memoria.h"
#include "semafor.h"
#include "missatge.h"

#include <stdbool.h>

int id_bustia_pal, id_bustia_pare; // Variables globals.
bool missatgeRebut = false;
int id_sem, op, dir;
int numeroPaleta;
char mis[2];
int mem_busties, *p_mem_busties;


/**
 * Funcio que comproba si rep un missatge en la bustia de la paleta per fer revisio.
*/
void *comprobarMiss() {
  int missatge;
  while(1) {
    missatge = receiveM(p_mem_busties[numeroPaleta], mis);
    if (missatge != 0) {
      missatgeRebut = true;
      waitS(id_sem);
      if (mis[0] == '1') {
        dir = 1; // derecha
        op = 1; // Direccion del movimiento es decir col + op 
      }
      if (mis[0] == '2') {
        dir = 2; // izquierda
        op = -1; // Direccion del movimiento es decir col + op 
      }
      signalS(id_sem);
    }
  }
  return 0;
}


/**
 * @brief Funció main que controla la paleta del ordinador.
*/
int main (int n_args, char *ll_args[]) {
  if (n_args < 5) {
    printf("Error en els arguments d'entrada. Exemple d'ús: pal_ord3 0 0 0 0\n");
    exit(EXIT_FAILURE);
  }

    typedef struct { // Estructura de datos compartidos mediante un mapa de memoria. 
      int tec; 
      int cont;
      int moviments;
      int moviments_inicials;
      int moviments_infinits;
      int retard;
      int l_pal;
      int n_fil;
      int n_col;
    } DadesCompartides;


    typedef struct { // Estructura de datos compartidos por las paletas mediante un mapa de memoria. 
      int ipo_pf, ipo_pc;      	/* posicio del la paleta de l'ordinador */
      float v_pal;			/* velocitat de la paleta del programa */
      float pal_ret;			/* percentatge de retard de la paleta */
      float po_pf;	/* pos. vertical de la paleta de l'ordinador, en valor real */
    } Fila;

    // Obtenemos todos los punteros del mapeo de memoria. 
    int id_mem_dadesCompartides = atoi(ll_args[1]);  
    int id_mem_matrizMovimientosP = atoi(ll_args[2]);
    int id_mem_matrizPaletas = atoi(ll_args[3]);
    numeroPaleta = atoi(ll_args[4]); // Índex de la paleta.
    int pWin = atoi(ll_args[5]);
    void *pWinP = map_mem(pWin);
    DadesCompartides* dades = map_mem(id_mem_dadesCompartides);
    int* matrizMovimientosPaletas = map_mem(id_mem_matrizMovimientosP);
    Fila* matrizPaletas = map_mem(id_mem_matrizPaletas);
    int f_h;

    // Preparem el semafor 
    id_sem = atoi(ll_args[6]);

    // Preparem les busties
    id_bustia_pare = atoi(ll_args[7]);
    mem_busties = atoi(ll_args[8]);
    p_mem_busties = map_mem(mem_busties);

    // Creem el thread que verifica si hi ha missatge
    pthread_t missatge;
    pthread_create(&missatge, NULL, comprobarMiss, NULL);
  
  // Fem un winset per permetre la escritura de pantalla per a les paletes 
  win_set(pWinP, dades->n_fil, dades->n_col);  

  // INICI CODI DE PALETES.
  while ((dades->tec != TEC_RETURN) && (dades->cont == -1) && ((dades->moviments > 0) || dades->moviments == -1 || dades->moviments_infinits == 1)) {
    win_retard(dades->retard * matrizPaletas[numeroPaleta].pal_ret); // Retard de moviment 

    if (missatgeRebut) { // Verifiquem si hem rebut un missatge. 
      waitS(id_sem); // Esperem al semafor. 
      if(dir == 1 || dir == 2) { // Si la direccio es 1(dreta) o 2(esquerra)
        bool enviar = false;
        int fila = matrizPaletas[numeroPaleta].ipo_pf;
        int columna = matrizPaletas[numeroPaleta].ipo_pc;
        int j = 0;

        while(j < dades->l_pal && !enviar) { // Mirem si tenim alguna paleta al lateral on es va a fer el moviment
          if (matrizPaletas[j].ipo_pf == fila && matrizPaletas[j].ipo_pc == (columna+op)) {
            enviar = true;
          } else {
            j++; // J es la paleta en si, no un index, es a dir, mirem si la paleta 0 1 2 3 N esta al nostre lateral pegada a la que estem revisant. 
          }
        }

        if (!enviar) { // Si no la trobem 
          if((dir == 1 && ((columna + 1) == (dades->n_col-1))) || (dir == 2 && ((columna - 1) == 4))) { // Si estem al final del tauler-1 o al inici del tauler+4
            for (int i = 0; i < dades->l_pal; i++) { 
              win_escricar(fila, columna, ' ', NO_INV); // Eliminem la paleta
              fila++;
            }
            pthread_cancel(missatge); // Tanquem tant els threads com el proces de la paleta. 
            pthread_join(missatge, NULL);
            signalS(id_sem);
            exit(0);
          } else {
            for (int i = 0; i < dades->l_pal; i++) // Si no estem al final del tauler
            {
              win_escricar(fila, columna, ' ', NO_INV); // Eliminem la paleta de la pantalla
              win_escricar(fila, (columna + op), '1' + numeroPaleta, INVERS); // La escribim en la nova posicio
              fila++; 
            }
            matrizPaletas[numeroPaleta].ipo_pc = columna + op; // Augmentem la posicio horizontal (columna)
          }
        } else { // Si es te que enviar, enviem un misatge a la altra paleta per fer el desplazament.
          char miss[2];
          sprintf(miss, "%c", mis[0]); // Preparem el missatge, en resum miss_enviar = mis rebut. 
          sendM(p_mem_busties[j], miss, 2); // Enviem el missatge. 
        }
      }
      signalS(id_sem); // Enviem señal al semafor
      missatgeRebut = false; // Tornem a dir que no hem rebut cap missatge. 
    }

    f_h =matrizPaletas[numeroPaleta].po_pf + matrizPaletas[numeroPaleta].v_pal;		/* posicio hipotetica de la paleta */
    if (f_h != matrizPaletas[numeroPaleta].ipo_pf)	/* si pos. hipotetica no coincideix amb pos. actual */
    {
      waitS(id_sem);
      if (matrizPaletas[numeroPaleta].v_pal > 0.0)			/* verificar moviment cap avall */
      {
        
        if (win_quincar(f_h+dades->l_pal-1,matrizPaletas[numeroPaleta].ipo_pc) == ' ')   /* si no hi ha obstacle */
        {
          
          win_escricar(matrizPaletas[numeroPaleta].ipo_pf,matrizPaletas[numeroPaleta].ipo_pc,' ',NO_INV);      /* esborra primer bloc */
          
          matrizPaletas[numeroPaleta].po_pf += matrizPaletas[numeroPaleta].v_pal; matrizPaletas[numeroPaleta].ipo_pf = matrizPaletas[numeroPaleta].po_pf;		/* actualitza posicio */
          
          win_escricar(matrizPaletas[numeroPaleta].ipo_pf+dades->l_pal-1,matrizPaletas[numeroPaleta].ipo_pc,'1'+numeroPaleta,INVERS); /* impr. ultim bloc */
          if (dades->moviments > 0){
            dades->moviments--;    /* he fet un moviment de la paleta */
            matrizMovimientosPaletas[numeroPaleta]++; // Actualitzem el numero de moviments de la paleta.
          }
          
        } else {
          /* si hi ha obstacle, canvia el sentit del moviment */
          
          matrizPaletas[numeroPaleta].v_pal = -matrizPaletas[numeroPaleta].v_pal;
        }
        signalS(id_sem);
      }
      else			/* verificar moviment cap amunt */
      { 
    if (win_quincar(f_h,matrizPaletas[numeroPaleta].ipo_pc) == ' ')        /* si no hi ha obstacle */
    {
      
      win_escricar(matrizPaletas[numeroPaleta].ipo_pf+dades->l_pal-1,matrizPaletas[numeroPaleta].ipo_pc,' ',NO_INV); /* esbo. ultim bloc */
      
      matrizPaletas[numeroPaleta].po_pf += matrizPaletas[numeroPaleta].v_pal; matrizPaletas[numeroPaleta].ipo_pf = matrizPaletas[numeroPaleta].po_pf;		/* actualitza posicio */
      
      win_escricar(matrizPaletas[numeroPaleta].ipo_pf,matrizPaletas[numeroPaleta].ipo_pc,'1'+numeroPaleta,INVERS);	/* impr. primer bloc */
      if (dades->moviments > 0){
        dades->moviments--;    /* he fet un moviment de la paleta */
        matrizMovimientosPaletas[numeroPaleta]++; // Actualitzem el numero de moviments de la paleta.
      }
    }
    else		/* si hi ha obstacle, canvia el sentit del moviment */
    {
      matrizPaletas[numeroPaleta].v_pal = -matrizPaletas[numeroPaleta].v_pal;
    }
    signalS(id_sem);
    }
    }
    else {
      matrizPaletas[numeroPaleta].po_pf += matrizPaletas[numeroPaleta].v_pal;
      }	/* actualitza posicio vertical real de la paleta */
  }
  pthread_cancel(missatge); // Directament ens carreguem el thread, no podem fer-ho amb join, pos el proces es queda bloquejat escoltant la cua de missatges. 
  pthread_join(missatge, NULL);
  return 0;
}