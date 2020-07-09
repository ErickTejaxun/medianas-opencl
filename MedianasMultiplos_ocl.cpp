#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>
#include <CL/cl.h>


using namespace std;

#define MAX_SOURCE_SIZE (0x100000)
void CodigoError(cl_int err);



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
		CodigoError(error);
		return error;
	}	

	cl_uint num_devices = 0;
	error = clGetDeviceIDs (entorno->plataformas[0], CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices);
	if(error!=CL_SUCCESS)
	{
		printf("Error al obtener dispositivos \n");
		CodigoError(error);
		return error;
	}	
	printf("Se encontraron %i dispositivos \n",num_devices);

	/*Preparar espacio de memoria para los dispositivos*/	
	entorno->dispositivos = (cl_device_id *) malloc (num_devices * sizeof (cl_device_id));

	/*Obtenemos información de los dispositivos */
	error = clGetDeviceIDs (entorno->plataformas[0], CL_DEVICE_TYPE_CPU, num_devices, entorno->dispositivos, NULL);	
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);
		return error;
	}	
	/*Creamos el contexto*/
	entorno->contexto = clCreateContext (NULL, num_devices, entorno->dispositivos, NULL, NULL, &error);
	if(error!=CL_SUCCESS)
	{		
		printf("error al crear contexto \n");
		CodigoError(error);
		return error;
	}
	/*Creamos la cola de comandos*/
	entorno->cola = clCreateCommandQueue (entorno->contexto, entorno->dispositivos[0], 0, &error);	    
	if(error!=CL_SUCCESS)
	{		
		printf("error al crear cola de comandos. \n");
		CodigoError(error);
		return error;
	}

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
	//printf("Kernel\n:%s",source_str);
	printf("Tamaino fuente: %d\n", (int)tamanio_fuente);
	fclose(fp);
	entorno->programa = clCreateProgramWithSource(entorno->contexto, 1, (const char **)&source_str,(const size_t *)&tamanio_fuente, &error);
	if(error!=CL_SUCCESS)
	{		
		printf("Error al crear el programa.");
		CodigoError(error);
		return error;
	}		


	error = clBuildProgram(entorno->programa, 1, &entorno->dispositivos[0], NULL, NULL, NULL);
	if(error!=CL_SUCCESS)
	{			
		printf("Error al construir el programa\n")	;
		CodigoError(error);	
		return error;
	}

	/*Ahora creamos el kernel*/
	entorno->kernel = clCreateKernel (entorno->programa, "ordenar", &error);
	if(error!=CL_SUCCESS)
	{			
		printf("Error al crear el kernel\n")	;
		CodigoError(error);	
		return error;
	}	
}

cl_int LiberarEntornoOCL(EntornoOCL_t *entorno) 
{
	clReleaseKernel (entorno->kernel);
	clReleaseProgram (entorno->programa);
	clReleaseCommandQueue (entorno->cola);
	/*Aqui vamos a liberar los buffers*/
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
	cl_int error;	
	cl_mem buffer;
	size_t data_size = sizeof (int) * 1000;	
		


    /* define index space (number of work-items), here only use 1 work group */
    size_t global_work_size[1];

    /* The number of work-items in work-group is `num_elements` */
    global_work_size[0] = columnas;	

    

    //clEnqueueReadBuffer (entorno.cola, , CL_TRUE, 0, data_size, c, 0, NULL, NULL);

    /*(__global int *columnas,
     __global int *m,
	__global int *v, 
	__global int *debug, 
	__global int *ne
	)*/

	cl_mem buffer_m, buffer_v, buffer_debug, buffer_columnas, buffer_ne,buffer_fila;
    	buffer_columnas = clCreateBuffer (entorno.contexto, CL_MEM_READ_ONLY, 1, NULL, NULL);
	buffer_m = clCreateBuffer (entorno.contexto, CL_MEM_READ_ONLY, columnas*columnas-1, NULL, NULL);
	buffer_v = clCreateBuffer (entorno.contexto, CL_MEM_READ_WRITE, columnas-1, NULL, NULL);
	buffer_debug = clCreateBuffer (entorno.contexto, CL_MEM_READ_ONLY, 1, NULL, NULL);	
	buffer_ne = clCreateBuffer (entorno.contexto, CL_MEM_READ_ONLY, 1, NULL, NULL);	
	buffer_fila = clCreateBuffer (entorno.contexto, CL_MEM_READ_ONLY, columnas, NULL, NULL);	
	cl_event eventos[6];

	error = clEnqueueWriteBuffer (entorno.cola, buffer_columnas, CL_TRUE, 0, 1, &columnas, 0, NULL, &eventos[0]);
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);		
	}

	error = clEnqueueWriteBuffer (entorno.cola, buffer_m, CL_TRUE, 0, columnas*columnas-1, m, 1, &eventos[0], &eventos[1]);
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);		
	}	
	error = clEnqueueWriteBuffer (entorno.cola, buffer_v, CL_TRUE, 0, columnas-1, v,  1, &eventos[1], &eventos[2]);
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);		
	}	
	error = clEnqueueWriteBuffer (entorno.cola, buffer_debug, CL_TRUE, 0, 1, &debug,  1, &eventos[2], &eventos[3]);
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);		
	}	
	error = clEnqueueWriteBuffer (entorno.cola, buffer_ne, CL_TRUE, 0, 1, &ne,  1, &eventos[3], &eventos[4]);
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);		
	}	

	int *fila__ = new int[columnas]; 			// Columna temporal 
	error = clEnqueueWriteBuffer (entorno.cola, buffer_fila, CL_TRUE, 0, 1, &fila__,  1, &eventos[4], &eventos[5]);
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);		
	}	

	error =  clSetKernelArg (entorno.kernel, 0, sizeof (cl_mem), &buffer_columnas);
	if(error!=CL_SUCCESS)
	{		
		CodigoError(error);		
	}		
	clSetKernelArg (entorno.kernel, 1, sizeof (cl_mem), &buffer_m);
	clSetKernelArg (entorno.kernel, 2, sizeof (cl_mem), &buffer_v);	
	clSetKernelArg (entorno.kernel, 3, sizeof (cl_mem), &buffer_debug);	
	clSetKernelArg (entorno.kernel, 4, sizeof (cl_mem), &buffer_ne);
	clSetKernelArg (entorno.kernel, 5, sizeof (cl_mem), &buffer_fila);

	printf("Empezando ejecuciión\n");
	

	//error =clEnqueueTask(entorno.cola,entorno.kernel,4,  eventos, NULL );
	error =clEnqueueNDRangeKernel (entorno.cola,entorno.kernel, 1, NULL, global_work_size, NULL, 6, eventos, NULL );
	//clEnqueueTask(entorno.cola, entorno.kernel, 0, NULL,NULL);
	if(error!=CL_SUCCESS)
	{
		printf("Error al ejecutar el kernel\n");
		CodigoError(error);
	}
	clFinish(entorno.cola);
	printf("Fin ejecuciión\n");
	//escribirmatrix(columnas,m);

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



void CodigoError(cl_int err) {
	switch (err) {
		case CL_SUCCESS: printf("Ejecuci�n correcta\n"); break;
		case CL_BUILD_PROGRAM_FAILURE: printf("CL_BUILD_PROGRAM_FAILURE\n"); break;
		case CL_COMPILER_NOT_AVAILABLE: printf("CL_COMPILER_NOT_AVAILABLE\n"); break;
		case CL_DEVICE_NOT_AVAILABLE: printf("CL_DEVICE_NOT_AVAILABLE \n"); break;
		case CL_DEVICE_NOT_FOUND: printf("CL_DEVICE_NOT_FOUND\n"); break;
		case CL_INVALID_ARG_INDEX : printf("CL_INVALID_ARG_INDEX \n"); break;
		case CL_INVALID_ARG_SIZE : printf("CL_INVALID_ARG_SIZE \n"); break;
		case CL_INVALID_ARG_VALUE: printf("CL_INVALID_ARG_VALUE\n"); break;
		case CL_INVALID_BUFFER_SIZE : printf("CL_INVALID_BUFFER_SIZE \n"); break;
		case CL_INVALID_BUILD_OPTIONS: printf("CL_INVALID_BUILD_OPTIONS\n"); break;
		case CL_INVALID_COMMAND_QUEUE : printf("CL_INVALID_COMMAND_QUEUE \n"); break;
		case CL_INVALID_CONTEXT: printf("CL_INVALID_CONTEXT\n"); break;
		case CL_INVALID_DEVICE_TYPE: printf("CL_INVALID_DEVICE_TYPE\n"); break;
		case CL_INVALID_EVENT: printf("CL_INVALID_EVENT\n"); break;
		case CL_INVALID_EVENT_WAIT_LIST : printf("CL_INVALID_EVENT_WAIT_LIST \n"); break;
		case CL_INVALID_GLOBAL_WORK_SIZE : printf("CL_INVALID_GLOBAL_WORK_SIZE \n"); break;
		case CL_INVALID_HOST_PTR : printf("CL_INVALID_HOST_PTR \n"); break;
		case CL_INVALID_KERNEL: printf("CL_INVALID_KERNEL \n"); break;
		case CL_INVALID_KERNEL_ARGS : printf("CL_INVALID_KERNEL_ARGS \n"); break;
		case CL_INVALID_KERNEL_NAME: printf("CL_INVALID_KERNEL_NAME\n"); break;
		case CL_INVALID_MEM_OBJECT : printf("CL_INVALID_MEM_OBJECT \n"); break;
		case CL_INVALID_OPERATION : printf("\n"); break;
		case CL_INVALID_PLATFORM: printf("CL_INVALID_PLATFORM\n"); break;
		case CL_INVALID_PROGRAM : printf("CL_INVALID_PROGRAM \n"); break;
		case CL_INVALID_PROGRAM_EXECUTABLE : printf("CL_INVALID_PROGRAM_EXECUTABLE\n"); break;
		case CL_INVALID_QUEUE_PROPERTIES: printf("CL_INVALID_QUEUE_PROPERTIES\n"); break;
		case CL_INVALID_VALUE: printf("CL_INVALID_VALUE\n"); break;
		case CL_INVALID_WORK_DIMENSION : printf("CL_INVALID_WORK_DIMENSION \n"); break;
		case CL_INVALID_WORK_GROUP_SIZE : printf("CL_INVALID_WORK_GROUP_SIZE \n"); break;
		case CL_INVALID_WORK_ITEM_SIZE : printf("CL_INVALID_WORK_ITEM_SIZE \n"); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE : printf("CL_MEM_OBJECT_ALLOCATION_FAILURE \n"); break;
		case CL_OUT_OF_HOST_MEMORY: printf("CL_OUT_OF_HOST_MEMORY\n"); break;
		case CL_OUT_OF_RESOURCES: printf("CL_OUT_OF_RESOURCES\n"); break;
		case CL_PROFILING_INFO_NOT_AVAILABLE : printf("CL_PROFILING_INFO_NOT_AVAILABLE \n"); break;
		default: printf("C�digo de error no contemplado\n"); break;
	}
}