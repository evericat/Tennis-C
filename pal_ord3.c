

#include "winsuport2.h"


/* funcio per moure la paleta de l'ordinador; la paleta es mou en funcio del retard*/
void *mou_paleta_ordinador(void *index) {
  int f_h;
 /* char rh,rv,rd; */
  while ((tec != TEC_RETURN) && (cont == -1) && ((moviments > 0) || moviments == -1 || moviments_infinits == 1)) {
    /**
      * Per no posar sempre el mateix comentari: 
      * pthread_mutex_lock(&sem); // Bloqueja el semafor
      * // Codi que volem protegir
      * pthread_mutex_unlock(&sem); // Desbloqueja el semafor
    */
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