

#include "winsuport2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "memoria.h"


/**
 * @brief Funció que mou la paleta de l'ordinador.
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


 /* char rh,rv,rd; */
  
  // Fem un winset 

  win_set(pWinP, dades->n_fil, dades->n_col); // Posem la escritura de pantalla també per a les paletes del ordinador. 

  while ((dades->tec != TEC_RETURN) && (dades->cont == -1) && ((dades->moviments > 0) || dades->moviments == -1 || dades->moviments_infinits == 1)) {
    win_retard(dades->retard * matrizPaletas[index].pal_ret);
    f_h =matrizPaletas[index].po_pf + matrizPaletas[index].v_pal;		/* posicio hipotetica de la paleta */
    if (f_h != matrizPaletas[index].ipo_pf)	/* si pos. hipotetica no coincideix amb pos. actual */
    {
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
    }
    }
    else {
      matrizPaletas[index].po_pf += matrizPaletas[index].v_pal;
      }	/* actualitza posicio vertical real de la paleta */
  }
  return 0;
}