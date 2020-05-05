#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>
#include <CL/cl.h>

using namespace std;

//Inicializa la matriz m de tamaño nxn en el rango (lv, uv)
void inicializematrix(int n,int *m,int lv,int uv)
{
	for(int i=0;i<n;i++)
		m[i]=(int) (((1.*rand())/RAND_MAX)*(uv-lv)+lv);
}

//Muestra la matriz m de tamaño nxn
void escribirmatrix(int n,int *m){
	for (int i=0; i<n+1; i++)
	{
		for(int j=0; j<n; j++)
			cout << m[i*n+j]<<" ";
		cout<<endl;
	}
	cout <<endl;
}

//Muestra el vector v de tamaño n
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

int ObtenerParametros(int argc, char *argv[], int *debug, int *num_workitems, int *workitems_por_workgroups) {
	int i;
	*debug=0;
	*num_workitems=0;
	*workitems_por_workgroups=0;
	if (argc<2)
		return 0;
	for (i=2; i<argc;) {
		if (strcmp(argv[i], "-d")==0) {
			*debug=1;
			i++;
		}
		else if (strcmp(argv[i], "-wi")==0) {
			i++;
			if (i==argc)
				return 0;
			*num_workitems=atoi(argv[i]);
			i++;
			if (*num_workitems<=0)
				return 0;
		}
		else if (strcmp(argv[i], "-wi_wg")==0) {
			i++;
			if (i==argc)
				return 0;
			*workitems_por_workgroups=atoi(argv[i]);
			i++;
			if (*workitems_por_workgroups<=0)
				return 0;
		}
		else
			return 0;
	}
	return 1;
}

typedef struct {
	cl_platform_id *plataformas;
	cl_device_id *dispositivos;
	cl_context contexto;
	cl_command_queue cola;
	cl_program programa;
	cl_kernel kernel;
} EntornoOCL_t;

// **************************************************************************
// ***************************** IMPLEMENTACIÓN *****************************
// **************************************************************************
cl_int InicializarEntornoOCL(EntornoOCL_t *entorno) {

}

cl_int LiberarEntornoOCL(EntornoOCL_t *entorno) {

}

/*
columnas -> Número de columnas de la matriz. El número de filas de la matriz y tamaño del vector resultado es igual a columnas+1
m -> Matriz de tamaño nxn
v -> Vector resultado de tamaño columnas+1
debug -> Bandera para depuración
ne -> Número del experimento en ejecución (para depuración)
entorno -> Entorno OpenCL
*/
void ocl(int columnas,int *m,int *v, int debug, int ne, EntornoOCL_t entorno) {

}
// **************************************************************************
// *************************** FIN IMPLEMENTACIÓN ***************************
// **************************************************************************

/*
Recibirá los siguientes parámetros (los parámetros entre corchetes son opcionales): fichEntrada [-d] [-wi work_items] [-wi_wg workitems_por_workgroup]
fichEntrada -> Obligatorio. Fichero de entrada con los parámetros de lanzamiento de los experimentos
-d -> Opcional. Si se indica, se mostrarán por pantalla los valores iniciales, finales y tiempo de cada experimento
-wi work_items -> Opcional. Si se indica, se lanzarán tantos work items como se indique en work_items (para OpenCL)
-wi_wg workitems_por_workgroup -> Opcional. Si se indica, se lanzarán tantos work items en cada work group como se indique en WorkItems_por_WorkGroup (para OpenCL)
*/

int main(int argc,char *argv[]) {
	int i,
		debug,				   		// Indica si se desean mostrar los tiempos y resultados parciales de los experimentos
		num_workitems=0, 		   	// Número de work items que se utilizarán
		workitems_por_workgroups=0, // Número de work items por cada work group que se utilizarán
		num_problems,	  		   	// número de problemas
		columnas,	  			   	// Columnas de la matriz. Debe ser impar
		filas, 		  			   	// Filas de la matriz. Es igual al número de columnas más uno.
		*m,					   		// Matriz (será de tamaño nxn)
		*v,					   		// Vector solución (será de tamaño n)
		seed, 				   		// Semilla para la generación de números aleatorios
		lower_value,  			   	// Límite inferior para los valores de la matriz
		upper_value; 			   	// Límite superior para los valores de la matriz
	long long ti,tf,tt=0;		   	// Tiempos inicial, final y total
	EntornoOCL_t entorno;		   	// Entorno para el control de OpenCL
	FILE *f;					   	// Fichero de entrada con los datos de los experimentos
	
	if (!ObtenerParametros(argc, argv, &debug, &num_workitems, &workitems_por_workgroups)) {
		printf("Ejecución incorrecta\nEl formato correcto es %s fichEntrada [-d] [-wi work_items] [-wi_wg workitems_por_workgroup]\n", argv[0]);
		return 0;
	}

	InicializarEntornoOCL(&entorno);

	// Se lee el número de experimentos a realizar
	f = fopen(argv[1], "r");
	fscanf(f, "%d", &num_problems);
	
	for(i=0;i<num_problems;i++)
	{
		//Por cada experimento se leen
		fscanf(f, "%d",&columnas);		//El número de columnas. Debe ser impar
		filas=columnas+1;				//El número de filas es igual al número de columnas +1
		fscanf(f, "%d",&seed);			//La semilla para la generación de números aleatorios
		fscanf(f, "%d",&lower_value);	//El valor mínimo de para la generación de valores aleatorios de la matriz m (se generan entre lower_value+1 y upper_value-1)
		fscanf(f, "%d",&upper_value);	//El valor máximo de para la generación de valores aleatorios de la matriz m
		if (columnas%2==0) {
			printf("El experimento %d no puede realizarse por no ser la dimensión de la matriz impar\n", i);
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
		ocl(columnas,m,v,debug,i,entorno);
		tf=mseconds(); 
		tt+=tf-ti;

		if (debug)	{
			printf("\nTiempo del experimento %d: %Ld ms\n\nResultado del experimento %d:\n", i, tf-ti,i);
			escribirvector(filas,v);
		}
		delete[] v;
		delete[] m;
	}
	LiberarEntornoOCL(&entorno);
	fclose(f);
	printf("\nTiempo total de %d experimentos: %Ld ms\n", num_problems, tt);
	
	return 0;
}
