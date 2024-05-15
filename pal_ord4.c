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


/**
 * Funcio que comproba si rep un missatge en la bustia de la paleta per fer revisio.
*/
void *comprobarMiss() {
  int missatge;
  char mis[2];
  while(1) {
    missatge = receiveM(id_bustia_pal, mis);
    if (missatge != 0) {
      missatgeRebut = true;
      fprintf(stderr, "%c", mis[0]);

      waitS(id_sem);
      if (mis[0] == '1') {
        dir = 1;
        op = 1;
      }
      if (mis[0] == '2') {
        dir = 2;
        op = -1;
      }
      signalS(id_sem);

    }
    missatge = 0;
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

    typedef struct {
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


    typedef struct {
      int ipo_pf, ipo_pc;      	/* posicio del la paleta de l'ordinador */
      float v_pal;			/* velocitat de la paleta del programa */
      float pal_ret;			/* percentatge de retard de la paleta */
      float po_pf;	/* pos. vertical de la paleta de l'ordinador, en valor real */
    } Fila;

    int id_shm = atoi(ll_args[1]);
    int id_shm_matrizMovimientosP = atoi(ll_args[2]);
    int id_shm_matrizPaletas = atoi(ll_args[3]);
    int index = atoi(ll_args[4]); // Índex de la paleta.
    int pWin = atoi(ll_args[5]);
    void *pWinP = map_mem(pWin);
    DadesCompartides* dades = map_mem(id_shm);
    int* matrizMovimientosPaletas = map_mem(id_shm_matrizMovimientosP);
    Fila* matrizPaletas = map_mem(id_shm_matrizPaletas);
    int f_h;

    // Preparem el semafor
    id_sem = atoi(ll_args[6]);

    // Preparem les busties
    id_bustia_pare = atoi(ll_args[7]);
    id_bustia_pal = atoi(ll_args[8]); // PASAR A MAPMEM PARA QUE TENGAN TODOS LOS BUZONES.

    // Creem el thread que verifica si hi ha missatge
    pthread_t missatge;
    pthread_create(&missatge, NULL, comprobarMiss, NULL);
    


 /* char rh,rv,rd; */
  
  // Fem un winset 

  win_set(pWinP, dades->n_fil, dades->n_col); // Posem la escritura de pantalla també per a les paletes del ordinador. 

  while ((dades->tec != TEC_RETURN) && (dades->cont == -1) && ((dades->moviments > 0) || dades->moviments == -1 || dades->moviments_infinits == 1)) {
    win_retard(dades->retard * matrizPaletas[index].pal_ret);

    if (missatgeRebut) {
      waitS(id_sem);
      if(dir == 1 || dir == 2) {
        bool enviar = false;
        int fila = matrizPaletas[index].ipo_pf;
        int columna = matrizPaletas[index].ipo_pc;
        int j = 0;

        while(j < dades->l_pal && !enviar) {
          if (matrizPaletas[index].ipo_pf == fila && matrizPaletas[index].ipo_pc == (columna+op)) {
            enviar = true;
          } else {
            j++;
          }
        }

        if (!enviar) {
          if(dir == 1 && ((columna + 1) == (dades->n_col-1))) {
            for (int i = 0; i < dades->l_pal; i++) {
              win_escricar(fila, columna, ' ', NO_INV);
              fila++;
            }
            signalS(id_sem);
            pthread_cancel(missatge);
            pthread_join(missatge, NULL);
            exit(0);
          } else {
            for (int i = 0; i < dades->l_pal; i++)
            {
              win_escricar(fila, columna, ' ', NO_INV);
              win_escricar(fila, (columna + op), '1' + index, INVERS);
              fila++;
            }
            matrizPaletas[index].ipo_pc = columna + op;
          }
        } else {
          char mis[2];
          sprintf(mis, "%c", mis[0]);
        }
      }
      signalS(id_sem);
      missatgeRebut = false;
    }

    f_h =matrizPaletas[index].po_pf + matrizPaletas[index].v_pal;		/* posicio hipotetica de la paleta */
    if (f_h != matrizPaletas[index].ipo_pf)	/* si pos. hipotetica no coincideix amb pos. actual */
    {
      waitS(id_sem);
      if (matrizPaletas[index].v_pal > 0.0)			/* verificar moviment cap avall */
      {
        
        if (win_quincar(f_h+dades->l_pal-1,matrizPaletas[index].ipo_pc) == ' ')   /* si no hi ha obstacle */
        {
          
          
          win_escricar(matrizPaletas[index].ipo_pf,matrizPaletas[index].ipo_pc,' ',NO_INV);      /* esborra primer bloc */
          
          matrizPaletas[index].po_pf += matrizPaletas[index].v_pal; matrizPaletas[index].ipo_pf = matrizPaletas[index].po_pf;		/* actualitza posicio */
          
          win_escricar(matrizPaletas[index].ipo_pf+dades->l_pal-1,matrizPaletas[index].ipo_pc,'1'+index,INVERS); /* impr. ultim bloc */
          if (dades->moviments > 0){
            dades->moviments--;    /* he fet un moviment de la paleta */
            matrizMovimientosPaletas[index]++; // Actualitzem el numero de moviments de la paleta.
          }
          
        } else {
          /* si hi ha obstacle, canvia el sentit del moviment */
          
          matrizPaletas[index].v_pal = -matrizPaletas[index].v_pal;
        }
        signalS(id_sem);
      }
      else			/* verificar moviment cap amunt */
      { 
    if (win_quincar(f_h,matrizPaletas[index].ipo_pc) == ' ')        /* si no hi ha obstacle */
    {
      
      
      win_escricar(matrizPaletas[index].ipo_pf+dades->l_pal-1,matrizPaletas[index].ipo_pc,' ',NO_INV); /* esbo. ultim bloc */
      
      matrizPaletas[index].po_pf += matrizPaletas[index].v_pal; matrizPaletas[index].ipo_pf = matrizPaletas[index].po_pf;		/* actualitza posicio */
      
      win_escricar(matrizPaletas[index].ipo_pf,matrizPaletas[index].ipo_pc,'1'+index,INVERS);	/* impr. primer bloc */
      if (dades->moviments > 0){
        dades->moviments--;    /* he fet un moviment de la paleta */
        matrizMovimientosPaletas[index]++; // Actualitzem el numero de moviments de la paleta.
      }
           
    }
    else		/* si hi ha obstacle, canvia el sentit del moviment */
    {
      
      matrizPaletas[index].v_pal = -matrizPaletas[index].v_pal;
    }
    signalS(id_sem);
    }
    }
    else {
      matrizPaletas[index].po_pf += matrizPaletas[index].v_pal;
      }	/* actualitza posicio vertical real de la paleta */
  }
  pthread_cancel(missatge); // Directament ens carreguem el thread, no podem fer-ho amb join, pos el proces es queda bloquejat escoltant la cua de missatges. 
  pthread_join(missatge, NULL);
  return 0;
}