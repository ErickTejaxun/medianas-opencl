#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>
#include <CL/cl.h>


using namespace std;

#define MAX_SOURCE_SIZE (0x100000)




//Inicializa la matriz m de tama�o nxn en el rango (lv, uv)
void inicializematrix(int n,int *m,int lv,int uv)
{
	for(int i=0;i<n;i++)
		m[i]=(int) (((1.*rand())/RAND_MAX)*(uv-lv)+lv);
}

//Muestra la matriz m de tama�o nxn
void escribirmatrix(int n,int *m){
	for (int i=0; i<n+1; i++)
	{
		for(int j=0; j<n; j++)
			cout << m[i*n+j]<<" ";
		cout<<endl;
	}
	cout <<endl;
}

//Muestra el vector v de tama�o n
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

typedef struct 
{
	cl_platform_id *plataformas;
	cl_device_id *dispositivos;
	cl_context contexto;
	cl_command_queue cola;
	cl_program programa;
	cl_kernel kernel;
} EntornoOCL_t;

// **************************************************************************
// ***************************** IMPLEMENTACI�N *****************************
// **************************************************************************
cl_int InicializarEntornoOCL(EntornoOCL_t *entorno) 
{
	cl_uint  num_platforms = 0;
	cl_int error;
	error = clGetPlatformIDs (0, NULL, &num_platforms);
	if(error!=CL_SUCCESS)
	{
		printf("Error al obtener plataforma\n");
		return error;
	}
	printf("Se encontraron %i plataformas\n",num_platforms);

	/*Obtenciòn de ids de plataformas*/
	entorno->plataformas = (cl_platform_id *) malloc (num_platforms * sizeof (cl_platform_id));
	error = clGetPlatformIDs (num_platforms, entorno->plataformas, NULL);
	if(error!=CL_SUCCESS)
	{
		printf("Error al obtener información de plataformas\n");
		return error;
	}	

	cl_uint num_devices = 0;
	error = clGetDeviceIDs (entorno->plataformas[0], CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices);
	if(error!=CL_SUCCESS)
	{
		printf("Error al obtener dispositivos \n");
		return error;
	}	
	printf("Se encontraron %i dispositivos \n",num_devices);

	/*Preparar espacio de memoria para los dispositivos*/	
	entorno->dispositivos = (cl_device_id *) malloc (num_devices * sizeof (cl_device_id));

	/*Obtenemos información de los dispositivos */
	error = clGetDeviceIDs (entorno->plataformas[0], CL_DEVICE_TYPE_CPU, num_devices, entorno->dispositivos, NULL);	

	/*Creamos el contexto*/
	entorno->contexto = clCreateContext (NULL, num_devices, entorno->dispositivos, NULL, NULL, NULL);

	/*Creamos la cola de comandos*/
	entorno->cola = clCreateCommandQueue (entorno->contexto, entorno->dispositivos[0], 0, NULL);	
    
    /*Ahora creamos el programa*/
	FILE *fp;	
	char fileName[] = "./ordenamiento_kernel.cl";
	char *source_str;
	size_t tamanio_fuente;	
	fp = fopen(fileName, "r");
	if (!fp) 
	{
		fprintf(stderr, "Fallo al cargar el kernel .\n");
		exit(1);
	}

	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	tamanio_fuente = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	printf("Tamaino fuente: %d\n", (int)tamanio_fuente);
	fclose(fp);
	entorno->programa = clCreateProgramWithSource(entorno->contexto, 1, (const char **)&source_str,(const size_t *)&tamanio_fuente, &error);

	/*Ahora creamos el kernel*/
	entorno->kernel = clCreateKernel (entorno->programa, "MedianasMultiplos", NULL);
}

cl_int LiberarEntornoOCL(EntornoOCL_t *entorno) 
{
	clReleaseKernel (entorno->kernel);
	clReleaseProgram (entorno->programa);
	clReleaseCommandQueue (entorno->cola);
	//clReleaseMemObject(buffer);    
	clReleaseContext (entorno->contexto);
	free (entorno->plataformas);
	free (entorno->dispositivos);
}

/*
columnas -> N�mero de columnas de la matriz. El n�mero de filas de la matriz y tama�o del vector resultado es igual a columnas+1
m -> Matriz de tama�o nxn
v -> Vector resultado de tama�o columnas+1
debug -> Bandera para depuraci�n
ne -> N�mero del experimento en ejecuci�n (para depuraci�n)
entorno -> Entorno OpenCL
*/
void ocl(int columnas,int *m,int *v, int debug, int ne, EntornoOCL_t entorno) 
{
	/*Aqui hay que hacer la reparticiòn del trabajo*/ 
}
// **************************************************************************
// *************************** FIN IMPLEMENTACI�N ***************************
// **************************************************************************

/*
Recibir� los siguientes par�metros (los par�metros entre corchetes son opcionales): fichEntrada [-d] [-wi work_items] [-wi_wg workitems_por_workgroup]
fichEntrada -> Obligatorio. Fichero de entrada con los par�metros de lanzamiento de los experimentos
-d -> Opcional. Si se indica, se mostrar�n por pantalla los valores iniciales, finales y tiempo de cada experimento
-wi work_items -> Opcional. Si se indica, se lanzar�n tantos work items como se indique en work_items (para OpenCL)
-wi_wg workitems_por_workgroup -> Opcional. Si se indica, se lanzar�n tantos work items en cada work group como se indique en WorkItems_por_WorkGroup (para OpenCL)
*/

int main(int argc,char *argv[]) {
	int i,
		debug,				   		// Indica si se desean mostrar los tiempos y resultados parciales de los experimentos
		num_workitems=0, 		   	// N�mero de work items que se utilizar�n
		workitems_por_workgroups=0, // N�mero de work items por cada work group que se utilizar�n
		num_problems,	  		   	// n�mero de problemas
		columnas,	  			   	// Columnas de la matriz. Debe ser impar
		filas, 		  			   	// Filas de la matriz. Es igual al n�mero de columnas m�s uno.
		*m,					   		// Matriz (ser� de tama�o nxn)
		*v,					   		// Vector soluci�n (ser� de tama�o n)
		seed, 				   		// Semilla para la generaci�n de n�meros aleatorios
		lower_value,  			   	// L�mite inferior para los valores de la matriz
		upper_value; 			   	// L�mite superior para los valores de la matriz
	long long ti,tf,tt=0;		   	// Tiempos inicial, final y total
	EntornoOCL_t entorno;		   	// Entorno para el control de OpenCL
	FILE *f;					   	// Fichero de entrada con los datos de los experimentos
	
	if (!ObtenerParametros(argc, argv, &debug, &num_workitems, &workitems_por_workgroups)) {
		printf("Ejecuci�n incorrecta\nEl formato correcto es %s fichEntrada [-d] [-wi work_items] [-wi_wg workitems_por_workgroup]\n", argv[0]);
		return 0;
	}

	InicializarEntornoOCL(&entorno);

	// Se lee el n�mero de experimentos a realizar
	f = fopen(argv[1], "r");
	fscanf(f, "%d", &num_problems);
	
	for(i=0;i<num_problems;i++)
	{
		//Por cada experimento se leen
		fscanf(f, "%d",&columnas);		//El n�mero de columnas. Debe ser impar
		filas=columnas+1;				//El n�mero de filas es igual al n�mero de columnas +1
		fscanf(f, "%d",&seed);			//La semilla para la generaci�n de n�meros aleatorios
		fscanf(f, "%d",&lower_value);	//El valor m�nimo de para la generaci�n de valores aleatorios de la matriz m (se generan entre lower_value+1 y upper_value-1)
		fscanf(f, "%d",&upper_value);	//El valor m�ximo de para la generaci�n de valores aleatorios de la matriz m
		if (columnas%2==0) {
			printf("El experimento %d no puede realizarse por no ser la dimensi�n de la matriz impar\n", i);
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
