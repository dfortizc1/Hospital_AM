#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <time.h>
#include <Random64.h>
#include <bases.h>
#include <trabajadores.h>
#include <dynamics.h>
#include <test.h>
#include <trace.h>
#include <reaction.h>
#include <other_functions.h>

typedef void(*reactions) (std::vector<grupo> &Val, std::vector<grupo> &Vba, Crandom &ran, trabajadores *altos, trabajadores *bajos, std::vector<lognormal_d> &dist, int agentI);

int main(void)
{
  //Creo los arreglos de cada tipo
  trabajadores altos[Na], bajos[Nb];

  //Creo los vectores en donde están las personas de cada estadío
  std::vector<grupo> vecal, vecba; //En el hospital

  //Creo el generador de semillas
  Crandom gseed(difftime(time(0),0));

  //Defino la cantidad de tiempo de la corrida
  int T = 720;
  double t;

  //Defino las variables del acordeón
  //double nu = myconstants["Nu"], delta = myconstants["Delta"];
  //unsigned int loops = T/(nu+delta);

  //Defino las variables para el testeo masivo
  unsigned int tests = (int)(N*MyCons.theta);
  double dt = MyCons.nu/((double)tests);

  //Defino la variable de tiempo propio de cada reacción(tj)
  double tj[14];

  //Defino el número de corridas
  unsigned int ensemble = 1e3;

  //Creo el arreglo de las funciones de reacción
  reactions react[14] = {reaction0, reaction1, reaction2, reaction3, reaction4, reaction5, reaction6, reaction7, reaction8, reaction9,
			 reaction10, reaction11, reaction12, reaction13};

  //Creo los arreglos para los argumentos de forma y orden de las distribuciones
  double proms[12] = {De, De, Dpl, Dpl, Dpg, Dpg, Dpl+Dil, Dpl+Dil, Dil, Dil, Dig, Dig};
  double d_shape, d_order, CoefVar = 3.9/5.2;
  d_shape = std::sqrt(std::log((CoefVar*CoefVar) + 1.0));
  std::vector<lognormal_d> dist;
  for(unsigned int i=0; i<12; i++){
    d_order = std::log(proms[i]) - 0.5*(d_shape*d_shape);
    lognormal_d my_dist(d_order, d_shape);
    dist.push_back(my_dist);
  }
  
  //Variables auxiliares
  std::vector<double> ti_in;
  unsigned int n1, n2;
  double aux, contador, promrec;
  unsigned int aux2, num;
  double ARi, ARh, ARc;

  std::ofstream fout;
  std::string name;

  name = "Results/Data/datos_" + nameCons + ".csv";
  fout.open(name);
  
  contador = 0;
  promrec = 0;
  num = 0;
  
  //Genero las corridas
  while(contador < ensemble){
    std::cout << '\r';
    std::cout << "Vamos en la simulacion numero " << contador << " de " << ensemble << ".";
    //Los convierto del tamaño que son
    vecal.resize(16);    vecba.resize(16);
    
    //Le asigno a cada entrada el índice como valor
    vecal[0].resize(Na);    vecba[0].resize(Nb);
    for(unsigned int j=0; j<Na; j++){vecal[0][j] = j;    altos[j].init();}
    for(unsigned int j=0; j<Nb; j++){vecba[0][j] = j;    bajos[j].init();}

    name = "Results/Data_Prevalencia/datos_" + nameCons + "_" + std::to_string(num) + ".csv";
    //name = "prueba.csv";
    //fout.open(name);
    //fout.close();
    
    //Inicio el tiempo
    t = 0.0;
    ARi = 0.0;
    ARh = 0.0;
    ARc = 0.0;
    
    //Imprimo los datos
    print_inf(vecal, vecba, t, name);
    
    //Inicio los tiempos propios de cada reacción
    for(unsigned int j=0; j<14; j++){tj[j] = 0.0;}
    
    while(t < T){
      //Región de testeo masivo
      aux = 0.0;
      n1 = 0;
      while(aux < MyCons.nu){
	//Obtengo el tiempo e índice de la reacción
	ti_in = contagio(vecal, vecba, gseed, t, tj);
	
	//Si se tiene el tiempo máximo como tiempo mínimo, entonces termino la simulación
	if(ti_in[0] == 1e6){break;}

	//Voy contado de que tipo fue el contagio
	if((int)ti_in[1] == 0 || (int)ti_in[1] == 1){
	  if((int)ti_in[2] == -2){ARc++;}
	  else if((int)ti_in[2] == -1){ARh++;}
	  else{ARi++;}
	}
	
	//Actualizo los tiempos de los estados que pueden transitar
	update_times_all(vecal, vecba, altos, bajos, ti_in[0]);
	
	//Actualizo los tiempos de los testeados masivamente que dan negativo
	update_massive_all(vecal, vecba, altos, bajos, ti_in[0]);
	
	//Actualizo los tiempos de los leves aislado
	if(MyCons.AisLev){result_lev_ais(vecal, vecba, altos, bajos, ti_in[0], gseed);}

	//Actualizo los tiempos de los rastreados
	trace_massive_all(vecal, vecba, altos, bajos, ti_in[0]);
	
	//Actualizo los tiempos de los testeados y hago el rastreo de los nuevos aislados
	main_trace(vecal, vecba, altos, bajos, ti_in[0], gseed);
	
	//Genero los tests masivos
	/* Cuento los sus, exp, pre, lev y recI */
	aux2 = vecal[0].size() + vecba[0].size() + vecal[3].size() + vecba[3].size() + vecal[3].size() + vecba[6].size();
	aux2 += vecal[9].size() + vecba[9].size() + vecal[13].size() + vecba[13].size();
	n2 = (int)(aux/dt);
	for(unsigned int k=n1; k<n2 && k<tests && 0<aux2; k++){
	  massive_reaction(vecal, vecba, gseed, altos, bajos);
	  aux2 = vecal[0].size() + vecba[0].size() + vecal[3].size() + vecba[3].size() + vecal[3].size() + vecba[6].size();
	  aux2 += vecal[9].size() + vecba[9].size() + vecal[13].size() + vecba[13].size();
	}
	n1 = n2;
	
	//Genero la reacción según el índice que acabo de obtener
	react[(int)ti_in[1]](vecal, vecba, gseed, altos, bajos, dist, ti_in[2]);
	
	//Sumo el tiempo de la reacción
	t += ti_in[0];
	aux += ti_in[0];
	
	//Imprimo los datos
	print_inf(vecal, vecba, t, name);
	
	//Borro el vector de tiempo e índice
	ti_in.clear();
      }
      
      //Muevo los testeados masivos a su respectivo lugar
      move_massive_all(vecal, vecba, altos, bajos);
      
      //Si el vector de tiempo e índice no se borró, es porque se rompió el ciclo
      if(ti_in.size() != 0){break;}
      
      //Región sin testeo masivo
      aux = 0.0;
      while(aux < MyCons.delta){
	//Obtengo el tiempo e índice de la reacción
	ti_in = contagio(vecal, vecba, gseed, t, tj);
	
	//Si se tiene el tiempo máximo como tiempo mínimo, entonces termino la simulación
	if(ti_in[0] == 1e6){break;}

	//Voy contado de que tipo fue el contagio
	if((int)ti_in[1] == 0 || (int)ti_in[1] == 1){
	  if((int)ti_in[2] == -2){ARc++;}
	  else if((int)ti_in[2] == -1){ARh++;}
	  else{ARi++;}
	}
	
	//Actualizo los tiempos de los estados que pueden transitar
	update_times_all(vecal, vecba, altos, bajos, ti_in[0]);

	//Actualizo los tiempos de los leves aislado
	if(MyCons.AisLev){result_lev_ais(vecal, vecba, altos, bajos, ti_in[0], gseed);}

	//Actualizo los tiempos de los rastreados
	trace_massive_all(vecal, vecba, altos, bajos, ti_in[0]);
	
	//Actualizo los tiempos de los testeados y hago el rastreo de los nuevos aislados
	main_trace(vecal, vecba, altos, bajos, ti_in[0], gseed);
	
	//Actualizo los tiempos de los testeados masivamente, y si ya cumplieron tiempo, los devuelvo
	tested_massive_all(vecal, vecba, altos, bajos, ti_in[0]);
	
	//Genero la reacción según el índice que acabo de obtener
	react[(int)ti_in[1]](vecal, vecba, gseed, altos, bajos, dist, ti_in[2]);
	
	//Sumo el tiempo de la reacción
	t += ti_in[0];
	aux += ti_in[0];
	
	//Imprimo los datos
        print_inf(vecal, vecba, t, name);
	
	//Borro el vector de tiempo e índice
	ti_in.clear();
      }
      
      //Si el vector de tiempo e índice no se borró, es porque se rompió el ciclo
      if(ti_in.size() != 0){break;}
    }
    ti_in.clear();

    /* Cuento los recI, recT y recA */
    promrec += vecal[13].size() + vecba[13].size() + vecal[14].size() + vecba[14].size() + vecal[15].size() + vecba[15].size();
    fout << vecal[13].size() + vecba[13].size() + vecal[14].size() + vecba[14].size() + vecal[15].size() + vecba[15].size() << '\t';
    fout << ARi << '\t' << ARh << '\t' << ARc << std::endl;

    /* Imprimo la Red */
    name = "Results/Data_Grafos/datos_" + nameCons + "_" + std::to_string(num) + ".csv";
    print_net(vecal, vecba, altos, bajos, name);
    
    //Borro los vectores
    vecal.clear();    vecba.clear();

    contador++;
    num++;
  }
  fout.close();
  
  promrec /= (contador*N);

  std::cout << "\nAR " << nameCons << ": " << promrec << std::endl;

  return 0;
}
