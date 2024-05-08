

#include "winsuport2.h"
#include <stdio.h>
#include <stdlib.h>
#include "memoria.h"


/**
 * @brief Funció que mou la paleta de l'ordinador.
*/

int main (int n_args, char *ll_args[]) {
  int f_h;

  typedef struct {
    int ipo_pf, ipo_pc;      	/* posicio del la paleta de l'ordinador */
    float v_pal;			/* velocitat de la paleta del programa */
    float pal_ret;			/* percentatge de retard de la paleta */
    float po_pf;	/* pos. vertical de la paleta de l'ordinador, en valor real */
  } Fila;

  Fila *matrizPaletas = ll_args[0];
  int *matrizMovimientosPaletas = ll_args[1];
  int *index = ll_args[2];

 /* char rh,rv,rd; */
  
  while ((tec != TEC_RETURN) && (cont == -1) && ((moviments > 0) || moviments == -1 || moviments_infinits == 1)) {
    win_retard(retard);
    f_h =matrizPaletas[*(int*)index].po_pf + matrizPaletas[*(int*)index].v_pal;		/* posicio hipotetica de la paleta */
    if (f_h != matrizPaletas[*(int*)index].ipo_pf)	/* si pos. hipotetica no coincideix amb pos. actual */
    {
      if (matrizPaletas[*(int*)index].v_pal > 0.0)			/* verificar moviment cap avall */
      {
        if (win_quincar(f_h+l_pal-1,matrizPaletas[*(int*)index].ipo_pc) == ' ')   /* si no hi ha obstacle */
        {
          win_escricar(matrizPaletas[*(int*)index].ipo_pf,matrizPaletas[*(int*)index].ipo_pc,' ',NO_INV);      /* esborra primer bloc */
          matrizPaletas[*(int*)index].po_pf += matrizPaletas[*(int*)index].v_pal; matrizPaletas[*(int*)index].ipo_pf = matrizPaletas[*(int*)index].po_pf;		/* actualitza posicio */
          win_escricar(matrizPaletas[*(int*)index].ipo_pf+l_pal-1,matrizPaletas[*(int*)index].ipo_pc,'1'+*(int*)index,INVERS); /* impr. ultim bloc */
                if (moviments > 0){
                  moviments--;    /* he fet un moviment de la paleta */
                  matrizMovimientosPaletas[*(int*)index]++; // Actualitzem el numero de moviments de la paleta.
                } 
        } else {
          /* si hi ha obstacle, canvia el sentit del moviment */
          matrizPaletas[*(int*)index].v_pal = -matrizPaletas[*(int*)index].v_pal;
        }
      }
      else			/* verificar moviment cap amunt */
      { 
    if (win_quincar(f_h,matrizPaletas[*(int*)index].ipo_pc) == ' ')        /* si no hi ha obstacle */
    {
      win_escricar(matrizPaletas[*(int*)index].ipo_pf+l_pal-1,matrizPaletas[*(int*)index].ipo_pc,' ',NO_INV); /* esbo. ultim bloc */
      matrizPaletas[*(int*)index].po_pf += matrizPaletas[*(int*)index].v_pal; matrizPaletas[*(int*)index].ipo_pf = matrizPaletas[*(int*)index].po_pf;		/* actualitza posicio */
      win_escricar(matrizPaletas[*(int*)index].ipo_pf,matrizPaletas[*(int*)index].ipo_pc,'1'+*(int*)index,INVERS);	/* impr. primer bloc */
            if (moviments > 0){
              moviments--;    /* he fet un moviment de la paleta */
              matrizMovimientosPaletas[*(int*)index]++; // Actualitzem el numero de moviments de la paleta.
            }     
    }
    else		/* si hi ha obstacle, canvia el sentit del moviment */
    {
      matrizPaletas[*(int*)index].v_pal = -matrizPaletas[*(int*)index].v_pal;
    }
    }
    }
    else {
      matrizPaletas[*(int*)index].po_pf += matrizPaletas[*(int*)index].v_pal; 
      }	/* actualitza posicio vertical real de la paleta */
  }
  free(index); // Liberem la memoria de la variable index.
  return NULL;
}