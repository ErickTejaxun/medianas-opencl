#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>

using namespace std;

//Inicializa la matriz m de tamaóo nxn en el rango (lv, uv)
void inicializematrix(int n,int *m,int lv,int uv)
{
	for(int i=0;i<n;i++)
		m[i]=(int) (((1.*rand())/RAND_MAX)*(uv-lv)+lv);
}

//Muestra la matriz m de tamaóo nxn
void escribirmatrix(int n,int *m){
	for (int i=0; i<n+1; i++)
	{
		for(int j=0; j<n; j++)
			cout << m[i*n+j]<<"\t ";
		cout<<endl;
	}
	cout <<endl;
}

//Muestra el vector v de tamaóo n
void escribirvector(int n,int *v)
{
	for(int i=0;i<n;i++)
		cout  <<v[i]<<" ";
	cout <<endl;
}

/*
c
c     mseconds - returns elapsed milliseconds since Jan 1st, 1970.
c
*/
long long mseconds(){
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec*1000 + t.tv_usec/1000;
}
 
int ObtenerParametros(int argc, char *argv[], int *debug) {
	int i;
	*debug=0;
	if (argc<2)
		return 0;
	for (i=2; i<argc;) {
		if (strcmp(argv[i], "-d")==0) {
			*debug=1;
			i++;
		}
		else
			return 0;
	}
	return 1;
}

// **************************************************************************
// ***************************** IMPLEMENTACIóN *****************************
// **************************************************************************
/*
Implementación secuencial
columnas -> Nómero de columnas de la matriz. El nómero de filas de la matriz y tamaóo del vector resultado es igual a columnas+1
m -> Matriz de tamaóo nxn
v -> Vector resultado de tamaóo columnas+1
debug -> Bandera para depuración
ne -> Nómero del experimento en ejecución (para depuración)
*/
void sec(int columnas,int *m,int *v, int debug, int ne) 
{
	printf("Comenzando ordenamiento matriz experimento %i\n",ne);
	int filas = columnas + 1;
		
	for(int i = 0 ; i < filas ; i++)
	{	
		int *inicio = m + columnas*i; // Inicio de la fila
		int *fila = new int[columnas]; 			// Columna temporal 
		for(int x = 0; x <columnas; x++)
		{
			fila[x] = *inicio;
			inicio++;
		}		

	
		/*Comienza el ordenamiento de la fila actual.*/
		for(int p = columnas / 2; p > 0 ; p = p/2)
		{							
			for(int q = p; q < columnas; q++)
			{				
				int tmp = fila[q]; // Valor temporal del valor p-esimo	
				int z = q;			
				for(z = q; z >= p && fila[z-p] > tmp; z = z- p)
				{
					fila[z] = fila[z - p];
				}
				fila[z] = tmp;				
			}
		}	

		/*Fila ordenada*/
		if(debug)
		{			
			printf("\n");
			for(int j = 0 ; j< columnas; j++)
			{
				printf("%i\t",fila[j]);
			}
		}				
		
		/*Ahora contamos el número de veces que se encuentra  mediana = fila[ columnas / 2 +1]*/			
		int mediana = fila[columnas/2];
				
		int *indiceV = m;  // Dirección de memoria del i-esímo campo de la matirz de resultados. 				
		for(int f = 0 ; f < (columnas)*filas; f++)
		{
			//printf("\n%i)  \t %i --- %i = %i\n",f, *indiceV, mediana,  *indiceV%mediana);			
			if(mediana == 0)
			{
				indiceV++;
				break;
			}
			/*Si es menor no */
			if((*indiceV%mediana)==0)
			{
				v[i]++;
			}						
			indiceV++;
		}				
	}
}

// **************************************************************************
// *************************** FIN IMPLEMENTACIóN ***************************
// **************************************************************************

/*
Recibiró los siguientes parómetros (los parómetros entre corchetes son opcionales): fichEntrada [-d]
fichEntrada -> Obligatorio. Fichero de entrada con los parómetros de lanzamiento de los experimentos
-d -> Opcional. Si se indica, se mostrarón por pantalla los valores iniciales, finales y tiempo de cada experimento
*/
int main(int argc,char *argv[]) {
	int debug,		  	  // Indica si se desean mostrar los tiempos y resultados parciales de los experimentos
		num_problems, 	  // Nómero de problemas
		columnas,	  	  // Columnas de la matriz. Debe ser impar
		filas, 		  	  // Filas de la matriz. Es igual al nómero de columnas mós uno.
		*m,			  	  // Matriz (seró de tamaóo nxn)
		*v,			  	  // Vector solución (seró de tamaóo n)
		seed, 		  	  // Semilla para la generación de nómeros aleatorios
		lower_value,  	  // Lómite inferior para los valores de la matriz
		upper_value; 	  // Lómite superior para los valores de la matriz
	long long ti,tf,tt=0; // Tiempos inicial, final y total
	FILE *f;			  // Fichero de entrada con los datos de los experimentos
	
	if (!ObtenerParametros(argc, argv, &debug)) {
		printf("Ejecución incorrecta\nEl formato correcto es %s fichEntrada [-d]\n", argv[0]);
		return 0;
	}

	// Se lee el nómero de experimentos a realizar
	f = fopen(argv[1], "r");
	fscanf(f, "%d", &num_problems);
	for(int i=0;i<num_problems;i++)
	{
		//Por cada experimento se leen
		fscanf(f, "%d",&columnas);		//El nómero de columnas. Debe ser impar
		filas=columnas+1;				//El nómero de filas es igual al nómero de columnas +1
		fscanf(f, "%d",&seed);			//La semilla para la generación de nómeros aleatorios
		fscanf(f, "%d",&lower_value);	//El valor mónimo de para la generación de valores aleatorios de la matriz m (se generan entre lower_value+1 y upper_value-1)
		fscanf(f, "%d",&upper_value);	//El valor móximo de para la generación de valores aleatorios de la matriz m
		if (columnas%2==0) {
			printf("El experimento %d no puede realizarse por no ser el nómero de columnas de la matriz impar\n", i+1);
			continue;
		}

		// Reserva de memoria para la matriz y el vector resultado
		m=new int[filas*columnas];
		v = new int[filas];

		srand(seed);
		inicializematrix(filas*columnas,m,lower_value,upper_value);
		if (debug) {
			printf("\nMatriz del experimento %d:\n", i);
			escribirmatrix(columnas,m);
		}

		ti=mseconds(); 
		sec(columnas,m,v,debug,i);
		tf=mseconds(); 
		tt+=tf-ti;

		if (debug)	{
			printf("\nTiempo del experimento %d: %Ld ms\n\nResultado del experimento %d:\n", i, tf-ti,i);
			escribirvector(filas,v);
		}
		delete[] v;
		delete[] m;
	}
	fclose(f);
	printf("\nTiempo total de %d experimentos: %Ld ms\n", num_problems, tt);
	
	return 0;
}
