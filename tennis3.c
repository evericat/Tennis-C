/*****************************************************************************/
/*									     */
/*				     Tennis0.c				     */
/*									     */
/*  Programa inicial d'exemple per a les practiques 2 i 3 de FSO.	     */
/*     Es tracta del joc del tennis: es dibuixa un camp de joc rectangular   */
/*     amb una porteria a cada costat, una paleta per l'usuari, una paleta   */
/*     per l'ordinador i una pilota que va rebotant per tot arreu; l'usuari  */
/*     disposa de dues tecles per controlar la seva paleta, mentre que l'or- */
/*     dinador mou la seva automaticament (amunt i avall). Evidentment, es   */
/*     tracta d'intentar col.locar la pilota a la porteria de l'ordinador    */
/*     (porteria de la dreta), abans que l'ordinador aconseguixi col.locar   */
/*     la pilota dins la porteria de l'usuari (porteria de l'esquerra).      */
/*									     */
/*  Arguments del programa:						     */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil n_col m_por l_pal					     */
/*		pil_pf pil_pc pil_vf pil_vc pil_ret			     */
/*		ipo_pf ipo_pc po_vf pal_ret				     */
/*									     */
/*     on 'n_fil', 'n_col' son les dimensions del taulell de joc, 'm_por'    */
/*     es la mida de les dues porteries, 'l_pal' es la longitud de les dues  */
/*     paletes; 'pil_pf', 'pil_pc' es la posicio inicial (fila,columna) de   */
/*     la pilota, mentre que 'pil_vf', 'pil_vc' es la velocitat inicial,     */
/*     pil_ret es el percentatge respecte al retard passat per paràmetre;    */
/*     finalment, 'ipo_pf', 'ipo_pc' indicara la posicio del primer caracter */
/*     de la paleta de l'ordinador, mentre que la seva velocitat vertical    */
/*     ve determinada pel parametre 'po_fv', i pal_ret el percentatge de     */
/*     retard en el moviment de la paleta de l'ordinador.		     */
/*									     */
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment de la pilota i la paleta de l'ordinador (en ms);   */
/*     el valor d'aquest parametre per defecte es 100 (1 decima de segon).   */
/*									     */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides en 'winsuport.o', les       */
/*     quals proporcionen una interficie senzilla per a crear una finestra   */
/*     de text on es poden imprimir caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				     */
/*									     */
/*	   $ gcc tennis0.c winsuport.o -o tennis0 -lcurses		     */
/*	   $ tennis0 fit_param [retard]					     */
/*									     */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  fitxer no accessible					     */
/*	3  ==>  dimensions del taulell incorrectes			     */
/*	4  ==>  parametres de la pilota incorrectes			     */
/*	5  ==>  parametres d'alguna de les paletes incorrectes		     */
/*	6  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*****************************************************************************/



#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include "memoria.h" // Arxiu de control de memoria compartida 
#include <pthread.h> // llibreria pthread.
#include <time.h> // Llibreria per al time del joc

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define MIN_FIL 7		/* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MIN_PAL 3
#define MIN_VEL -1.0
#define MAX_VEL 1.0
#define MIN_RET 0.0
#define MAX_RET 5.0
#define NUMMAXPALETAS 9

/* variables globals */
int m_por;	/* dimensions del taulell i porteries */
// int n_fil, n_col;
pid_t tpid[NUMMAXPALETAS];	/* identificadors dels fils de les paletes */

int ipu_pf, ipu_pc;      	/* posicio del la paleta d'usuari */



int ipil_pf, ipil_pc;		/* posicio de la pilota, en valor enter */
float pil_pf, pil_pc;		/* posicio de la pilota, en valor real */
float pil_vf, pil_vc;		/* velocitat de la pilota, en valor real*/
float pil_ret;			/* percentatge de retard de la pilota */

//int retard;		/* valor del retard de moviment, en mil.lisegons */
//int moviments;		/* numero max de moviments paletes per acabar el joc */
//int moviments_inicials; /* Numero de moviments inicials, util per fer calculs*/
//int moviments_infinits; /* Es una variable booleana, on indica si els moviments son infinits o no*/

//int tec; // Tecla que pulsa el usuari
//int cont = -1; // Contador actual.
int n_pal = 0;

typedef struct {
  int tec;
  int cont;
  int moviments;
  int moviments_inicials;
  int moviments_infinits;
  int retard;
  int l_pal;			/* longitud de les paletes */
  int n_fil;
  int n_col;
} DadesCompartides;

DadesCompartides *dades; // Les dades esencials del joc
int id_shm; // id de direcció de la memoria 

typedef struct {
    int ipo_pf, ipo_pc;      	/* posicio del la paleta de l'ordinador */
    float v_pal;			/* velocitat de la paleta del programa */
    float pal_ret;			/* percentatge de retard de la paleta */
    float po_pf;	/* pos. vertical de la paleta de l'ordinador, en valor real */
} Fila;

int id_shm_matrizPaletas; // Identificador de memoria la matriz de paletas. 
Fila* matrizPaletas; 

int id_shm_matrizMovimientosP;
int* matrizMovimientosPaletas;

int id_shm_retwin;
void* shared_mem_retwin;

pthread_mutex_t sem = PTHREAD_MUTEX_INITIALIZER;

/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins un fitxer de text, el nom del qual es passa per referencia en   */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris del principi del programa).		*/
void carrega_parametres(const char *nom_fit)
{
    int ipo_pf, ipo_pc;      	/* posicio del la paleta de l'ordinador */
    float v_pal;			/* velocitat de la paleta del programa */
    float pal_ret;			/* percentatge de retard de la paleta */
  FILE *fit;

  fit = fopen(nom_fit,"rt");		/* intenta obrir fitxer */
  if (fit == NULL)
  {	fprintf(stderr,"No s'ha pogut obrir el fitxer \'%s\'\n",nom_fit);
  	exit(2);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %d %d\n",&dades->n_fil,&dades->n_col,&m_por,&dades->l_pal);
  if ((dades->n_fil < MIN_FIL) || (dades->n_fil > MAX_FIL) ||
	(dades->n_col < MIN_COL) || (dades->n_col > MAX_COL) || (m_por < 0) ||
	 (m_por > dades->n_fil-3) || (dades->l_pal < MIN_PAL) || (dades->l_pal > dades->n_fil-3))
  {
	fprintf(stderr,"Error: dimensions del camp de joc incorrectes:\n");
	fprintf(stderr,"\t%d =< n_fil (%d) =< %d\n",MIN_FIL,dades->n_fil,MAX_FIL);
	fprintf(stderr,"\t%d =< n_col (%d) =< %d\n",MIN_COL,dades->n_col,MAX_COL);
	fprintf(stderr,"\t0 =< m_por (%d) =< n_fil-3 (%d)\n",m_por,(dades->n_fil-3));
	fprintf(stderr,"\t%d =< l_pal (%d) =< n_fil-3 (%d)\n",MIN_PAL,dades->l_pal,(dades->n_fil-3));
	fclose(fit);
	exit(3);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %f %f %f\n",&ipil_pf,&ipil_pc,&pil_vf,&pil_vc,&pil_ret);
  if ((ipil_pf < 1) || (ipil_pf > dades->n_fil-3) ||
	(ipil_pc < 1) || (ipil_pc > dades->n_col-2) ||
	(pil_vf < MIN_VEL) || (pil_vf > MAX_VEL) ||
	(pil_vc < MIN_VEL) || (pil_vc > MAX_VEL) ||
	(pil_ret < MIN_RET) || (pil_ret > MAX_RET))
  {
	fprintf(stderr,"Error: parametre pilota incorrectes:\n");
	fprintf(stderr,"\t1 =< ipil_pf (%d) =< n_fil-3 (%d)\n",ipil_pf,(dades->n_fil-3));
	fprintf(stderr,"\t1 =< ipil_pc (%d) =< n_col-2 (%d)\n",ipil_pc,(dades->n_col-2));
	fprintf(stderr,"\t%.1f =< pil_vf (%.1f) =< %.1f\n",MIN_VEL,pil_vf,MAX_VEL);
	fprintf(stderr,"\t%.1f =< pil_vc (%.1f) =< %.1f\n",MIN_VEL,pil_vc,MAX_VEL);
	fprintf(stderr,"\t%.1f =< pil_ret (%.1f) =< %.1f\n",MIN_RET,pil_ret,MAX_RET);
	fclose(fit);
	exit(4);
  }

  
  while(!feof(fit)) {
    fscanf(fit,"%d %d %f %f\n",&ipo_pf,&ipo_pc,&v_pal,&pal_ret);
    if ((ipo_pf < 1) || (ipo_pf+dades->l_pal > dades->n_fil-2) ||
    (ipo_pc < 5) || (ipo_pc > dades->n_col-2) ||
    (v_pal < MIN_VEL) || (v_pal > MAX_VEL) ||
    (pal_ret < MIN_RET) || (pal_ret > MAX_RET))
      {
    fprintf(stderr,"Error: parametres paleta ordinador incorrectes:\n");
    fprintf(stderr,"\t1 =< ipo_pf (%d) =< n_fil-l_pal-3 (%d)\n",ipo_pf,(dades->n_fil-dades->l_pal-3));
    fprintf(stderr,"\t5 =< ipo_pc (%d) =< n_col-2 (%d)\n",ipo_pc,(dades->n_col-2));
    fprintf(stderr,"\t%.1f =< v_pal (%.1f) =< %.1f\n",MIN_VEL,v_pal,MAX_VEL);
    fprintf(stderr,"\t%.1f =< pal_ret (%.1f) =< %.1f\n",MIN_RET,pal_ret,MAX_RET);
    fclose(fit);
    exit(5);
      }
      matrizPaletas[n_pal].ipo_pf = ipo_pf;
      matrizPaletas[n_pal].ipo_pc = ipo_pc;
      matrizPaletas[n_pal].v_pal = v_pal;
      matrizPaletas[n_pal].pal_ret = pal_ret;
      n_pal++;
  }
  fclose(fit);			/* fitxer carregat: tot OK! */
}


/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
int inicialitza_joc(void)
{
  int i, i_port, f_port, retwin;

  retwin = win_ini(&dades->n_fil,&dades->n_col,'+',INVERS);   /* intenta crear taulell */

  if (retwin < 0)       /* si no pot crear l'entorn de joc amb les curses */
  { fprintf(stderr,"Error en la creacio del taulell de joc:\t");
    switch (retwin)
    {   case -1: fprintf(stderr,"camp de joc ja creat!\n");
                 break;
        case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n");
 		 break;
        case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n");
                 break;
        case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n");
                 break;
     }
     return(retwin);
  }

  id_shm_retwin = ini_mem(retwin); // Inicialitzem la memoria compartida
  shared_mem_retwin = map_mem(id_shm_retwin); // Mapejem la memoria compartida
  win_set(shared_mem_retwin, dades->n_fil, dades->n_col); // Guardem la finestra en la memoria compartida
  
  
  i_port = dades->n_fil/2 - m_por/2;	    /* crea els forats de la porteria */
  if (dades->n_fil%2 == 0) i_port--;
  if (i_port == 0) i_port=1;
  f_port = i_port + m_por -1;
  for (i = i_port; i <= f_port; i++)
  {	win_escricar(i,0,' ',NO_INV);
	win_escricar(i,dades->n_col-1,' ',NO_INV);
  }
  ipu_pf = dades->n_fil/2; ipu_pc = 3;		/* inicialitzar pos. paletes */
  if (ipu_pf+dades->l_pal >= dades->n_fil-3) ipu_pf = 1;
  for (i=0; i< dades->l_pal; i++)	    /* dibuixar paleta inicialment */
  {	
    win_escricar(ipu_pf +i, ipu_pc, '0',INVERS);
  } 
  for (int i = 0; i < n_pal; i++)
  {
      for (int j = 0; j < dades->l_pal; j++) /* dibuixar paleta inicialment */
    {
      win_escricar(matrizPaletas[i].ipo_pf + j, matrizPaletas[i].ipo_pc, '1'+i, INVERS);
    }
    matrizPaletas[i].po_pf = matrizPaletas[i].ipo_pf; /* fixar valor real paleta ordinador */
  }
  while (win_quincar(ipil_pf, ipil_pc) != ' ') /* buscar posicio buida per la pilota */
  { ipil_pf = (rand() % (dades->n_fil-4)) + 2;
    ipil_pc = (rand() % (dades->n_col-4)) + 2;
  }
  pil_pf = ipil_pf; pil_pc = ipil_pc;	/* fixar valor real posicio pilota */
  win_escricar(ipil_pf, ipil_pc, '.',INVERS);	/* dibuix inicial pilota */
  /*
  sprintf(strin,"Tecles: \'%c\'-> amunt, \'%c\'-> avall, RETURN-> sortir.",
		TEC_AMUNT, TEC_AVALL);
  win_escristr(strin);*/
  win_update();    /* actualitza la pantalla */
  return(0);
}


/* funcio per moure la pilota; retorna un valor amb alguna d'aquestes	*/
/* possibilitats:							*/
/*	-1 ==> la pilota no ha sortit del taulell			*/
/*	 0 ==> la pilota ha sortit per la porteria esquerra		*/
/*	>0 ==> la pilota ha sortit per la porteria dreta		*/


void * moure_pilota(void * cap) {
  int f_h, c_h, result;
  char rh, rv, rd, pd;
  while ((dades->tec != TEC_RETURN) && (dades->cont == -1) && ((dades->moviments > 0) || dades->moviments == -1 || dades->moviments_infinits == 1))
  {
    /**
      * Per no posar sempre el mateix comentari: 
      *  // Bloqueja el semafor
      * // Codi que volem protegir
      *  // Desbloqueja el semafor
    */
    win_retard(dades->retard);
    f_h = pil_pf + pil_vf;      /* posicio hipotetica de la pilota */
    c_h = pil_pc + pil_vc;
    result = -1;        /* inicialment suposem que la pilota no surt */
    rh = rv = rd = pd = ' ';
    if ((f_h != ipil_pf) || (c_h != ipil_pc))
    {       /* si posicio hipotetica no coincideix amb la pos. actual */
      if (f_h != ipil_pf)     /* provar rebot vertical */
      {
        
        rv = win_quincar(f_h, ipil_pc);    /* veure si hi ha algun obstacle */
        
        if (rv != ' ')          /* si no hi ha res */
        {
          pil_vf = -pil_vf;       /* canvia velocitat vertical */
          f_h = pil_pf + pil_vf;  /* actualitza posicio hipotetica */
        }
      }
      if (c_h != ipil_pc)     /* provar rebot horitzontal */
      {
        
        rh = win_quincar(ipil_pf, c_h);    /* veure si hi ha algun obstacle */
        
        if (rh != ' ')          /* si no hi ha res */
        {
          pil_vc = -pil_vc;       /* canvia velocitat horitzontal */
          c_h = pil_pc + pil_vc;  /* actualitza posicio hipotetica */
        }
      }
      if ((f_h != ipil_pf) && (c_h != ipil_pc))    /* provar rebot diagonal */
      {
        
        rd = win_quincar(f_h, c_h);
        
        if (rd != ' ')              /* si no hi ha obstacle */
        {
          pil_vf = -pil_vf; pil_vc = -pil_vc;    /* canvia velocitats */
          f_h = pil_pf + pil_vf;
          c_h = pil_pc + pil_vc;      /* actualitza posicio entera */
        }
      }
      
      if (win_quincar(f_h, c_h) == ' ')    /* verificar posicio definitiva */
      {
                                           /* si no hi ha obstacle */
        win_escricar(ipil_pf, ipil_pc, ' ', NO_INV);    /* esborra pilota */
        pil_pf += pil_vf; pil_pc += pil_vc;
        ipil_pf = f_h; ipil_pc = c_h;       /* actualitza posicio actual */
        if ((ipil_pc > 0) && (ipil_pc <= dades->n_col)){
          /* si no surt */
          win_escricar(ipil_pf, ipil_pc, '.', INVERS); /* imprimeix pilota */
        }    
        else
        {
          result = ipil_pc;    /* codi de finalitzacio de partida */
        }
      }
       
    } else { // Si la pilota no es mou, actualitzem la posició de la pilota.
        pil_pf += pil_vf; pil_pc += pil_vc;
    }
    dades->cont = result; // Actualitzem la informació de la pilota.
  }
  return NULL;
}


void * mou_paleta_usuari(void * cap) {
    while ((dades->tec != TEC_RETURN) && (dades->cont == -1) && ((dades->moviments > 0) || dades->moviments == -1 || dades->moviments_infinits == 1)) {
        /**
         * Per no posar sempre el mateix comentari: 
         *  // Bloqueja el semafor
         * // Codi que volem protegir
         *  // Desbloqueja el semafor
        */
        win_retard(dades->retard);
        dades->tec = win_gettec();
        if (dades->tec != 0) {
          
          if ((dades->tec == TEC_AVALL) && (win_quincar(ipu_pf+dades->l_pal,ipu_pc) == ' '))
          {
              win_escricar(ipu_pf,ipu_pc,' ',NO_INV);	   /* esborra primer bloc */
              ipu_pf++;					   /* actualitza posicio */
              win_escricar(ipu_pf+dades->l_pal-1,ipu_pc,'0',INVERS); /* impri. ultim bloc */
              if (dades->moviments > 0) dades->moviments--;    /* he fet un moviment de la paleta */
              
          }
          if ((dades->tec == TEC_AMUNT) && (win_quincar(ipu_pf-1,ipu_pc) == ' '))
          {
              win_escricar(ipu_pf+dades->l_pal-1,ipu_pc,' ',NO_INV); /* esborra ultim bloc */
              ipu_pf--;					    /* actualitza posicio */
              win_escricar(ipu_pf,ipu_pc,'0',INVERS);	    /* imprimeix primer bloc */
              if (dades->moviments > 0) dades->moviments--;    /* he fet un moviment de la paleta */
          }
          /*if (tec == TEC_ESPAI) {
              win_escristr("==== PAUSA ====");
              tec = 0; 
              while (tec != TEC_ESPAI) {
                tec = win_gettec();
              }
              
          }*/
        }
    }
    return NULL;
}

void *actualitzar_pantalla(void *cap) {
    while ((dades->tec != TEC_RETURN) && (dades->cont == -1) && ((dades->moviments > 0) || dades->moviments == -1 || dades->moviments_infinits == 1)) {
        win_retard(dades->retard); 
        win_update();
    }
    return NULL;
}

void *mostra_informacio() {
    char strin[1024];
    time_t start_time = time(NULL);
    while ((dades->tec != TEC_RETURN) && (dades->cont == -1) && ((dades->moviments > 0) || dades->moviments == -1 || dades->moviments_infinits == 1)) {
      win_retard(dades->retard);
      time_t current_time = time(NULL);
      time_t elapsed_time = current_time - start_time;
      int minuts = elapsed_time / 60;
      int segons = elapsed_time % 60;
      if(dades->moviments_infinits == 0) {
        sprintf(strin,"Tecles: Amunt: \'%c\', Avall: \'%c\', RETURN-> sortir, MF: \'%d\', MR: \'%d\' T:\'%d:%d\'",TEC_AMUNT, TEC_AVALL, dades->moviments_inicials - dades->moviments, dades->moviments, minuts, segons);
         // Bloquejem el semafor perque anem a escriure en pantalla.
        win_escristr(strin);
         // Desbloquejem el semafor.
      } else {
        sprintf(strin,"Tecles: Amunt: \'%c\', Avall: \'%c\', RETURN-> sortir, MF: \'%d\', MR: INF T:\'%d:%d\'",TEC_AMUNT, TEC_AVALL, dades->moviments_inicials - dades->moviments, minuts, segons);
         // Bloquejem el semafor perque anem a escriure en pantalla.
        win_escristr(strin);
         // Desbloquejem el semafor.
      }
      if (dades->tec == TEC_ESPAI) {
              
              dades->tec = 0; 
              while (dades->tec != TEC_ESPAI) {
                win_retard(dades->retard);
                time_t current_time = time(NULL);
                time_t elapsed_time = current_time - start_time;
                int minuts = elapsed_time / 60;
                int segons = elapsed_time % 60;
                dades->tec = win_gettec();
                if(dades->moviments_infinits == 0) {
                  sprintf(strin,"Tecles: Amunt: \'%c\', Avall: \'%c\', RETURN-> sortir, MF: \'%d\', MR: \'%d\' T:\'%d:%d\'",TEC_AMUNT, TEC_AVALL, dades->moviments_inicials - dades->moviments, dades->moviments, minuts, segons);
                  win_escristr(strin);
                } else {
                  sprintf(strin,"Tecles: Amunt: \'%c\', Avall: \'%c\', RETURN-> sortir, MF: \'%d\', MR: INF T:\'%d:%d\'",TEC_AMUNT, TEC_AVALL, dades->moviments_inicials - dades->moviments, minuts, segons);
                  win_escristr(strin);
                }
              }
              
      }
    }
    return NULL;
}


/* programa principal				    */
int main(int n_args, const char *ll_args[])
{ 
  if ((n_args != 3) && (n_args !=4))
  {	fprintf(stderr,"Comanda: tennis0 fit_param moviments [retard]\n");
  	exit(1);
  }
  // Nomes carreguem les zones de memoria si tot ha anat be al inici. 
  id_shm = ini_mem(sizeof(DadesCompartides)); // Inicialitzem la memoria compartida
  dades = map_mem(id_shm); // Mapejem la memoria compartida
  dades->cont = -1; // Inicializamos el contador a -1. 

  id_shm_matrizPaletas = ini_mem(sizeof(Fila) * NUMMAXPALETAS); // Inicialitzem la memoria compartida
  matrizPaletas = map_mem(id_shm_matrizPaletas); // Mapejem la memoria compartida

  id_shm_matrizMovimientosP = ini_mem(sizeof(int) * NUMMAXPALETAS); // Inicialitzem la memoria compartida
  matrizMovimientosPaletas = map_mem(id_shm_matrizMovimientosP); // Mapejem la memoria compartida
  /**
   * Ara la carrega de parametres fara us de les 2 noves zones de memoria indicades, si fa el cas. Per als procesos. 
  */
  carrega_parametres(ll_args[1]); 
    dades->moviments=atoi(ll_args[2]);
    if (dades->moviments == 0) {
      dades->moviments = __INT_MAX__;
      dades->moviments_inicials = __INT_MAX__;
      dades->moviments_infinits = 1;
    } else {
      dades->moviments_inicials = dades->moviments;
      dades->moviments_infinits = 0;
    }
     

  if (n_args == 4) dades->retard = atoi(ll_args[3]);
  else dades->retard = 100; // Si no esta el retard, posem el retard a 100. 

  if (inicialitza_joc() !=0)    /* intenta crear el taulell de joc */
     exit(4);   /* aborta si hi ha algun problema amb taulell */

  //do				/********** bucle principal del joc **********/ 
  /*
  {	tec = win_gettec();
	if (tec != 0) mou_paleta_usuari(tec);
	mou_paleta_ordinador();
	cont = moure_pilota();
	win_retard(retard);
  } while ((tec != TEC_RETURN) && (cont==-1) && ((moviments > 0) || moviments == -1));*/

    pthread_t thread_pilota, thread_paleta_usuari, thread_time_moviments;
    pthread_t thread_actualizar_pantalla;
    pthread_mutex_init(&sem, NULL); // Inicialitzem el semafor.
    // Crear els fils
    pthread_create(&thread_pilota, NULL, moure_pilota, NULL);
    pthread_create(&thread_paleta_usuari, NULL, mou_paleta_usuari, NULL);
    pthread_create(&thread_time_moviments, NULL, mostra_informacio, NULL);
    pthread_create(&thread_actualizar_pantalla, NULL, actualitzar_pantalla, NULL);
    
    for (size_t i = 0; i < n_pal; i++)
    {
      matrizMovimientosPaletas[i] = 0; // INICIALIZAMOS TODOS LOS MOVIMENTOS A 0.
    }

    // Conversio de IDs a String. 
    char id_shm_str[32]; 
    char id_shm_matrizPaletas_str[32];
    char id_shm_matrizMovimientosP_str[32];
    char id_shm_retwin_str[32];
    sprintf(id_shm_str, "%d", id_shm);
    sprintf(id_shm_matrizPaletas_str, "%d", id_shm_matrizPaletas); 
    sprintf(id_shm_matrizMovimientosP_str, "%d", id_shm_matrizMovimientosP);
    sprintf(id_shm_retwin_str, "%d", id_shm_retwin);
    char index_str[12];
    for (int i = 0; i < n_pal; i++)
    {
      tpid[i] = fork(); // Creem el proces fill.
      if(tpid[i] == (pid_t) 0) {
        sprintf(index_str, "%d", i); // Convertim l'index a string.
        execlp("./pal_ord3", "./pal_ord3", id_shm_str, id_shm_matrizMovimientosP_str, id_shm_matrizPaletas_str, index_str, id_shm_retwin_str, (char*)0); // Creem el proces fill.
        fprintf(stderr, "No se ha pogut crear els fils de la paleta del ordinador, error!");
        exit(0);
      }
    }

    // Esperar que els fils acabin
    pthread_join(thread_paleta_usuari, NULL);
    pthread_join(thread_pilota, NULL);
    pthread_join(thread_time_moviments, NULL);
    pthread_join(thread_actualizar_pantalla, NULL);

    for (int i = 0; i < n_pal; i++)
    {
      waitpid(tpid[i], NULL, 0); // Esperem que acabi el proces fill.
    }

    // Destruim el semafor
    pthread_mutex_destroy(&sem); // Destruim el semafor.

    // Destruim les zones de memoria. 
    elim_mem(id_shm);
    elim_mem(id_shm_matrizMovimientosP);
    elim_mem(id_shm_matrizPaletas);
    elim_mem(id_shm_retwin);
    
  win_fi();

  if (dades->tec == TEC_RETURN) printf("S'ha aturat el joc amb la tecla RETURN!\n");
  else { if (dades->cont == 0 || dades->moviments == 0) printf("Ha guanyat l'ordinador!\n");
         else printf("Ha guanyat l'usuari!\n"); }

  for (int i = 0; i < n_pal; i++) {
    printf("Moviments de la paleta %d: %d\n", i, matrizMovimientosPaletas[i]);
  }
  return(0);
}
