//__kernel void ordenar(__global int *m, __global int *v, __global int *ne) { 
__kernel void ordenar(__global int *columnas, __global int *m,__global int *v, __global int *debug, __global int *ne){

    /*int columnas,int *m,int *v, int debug, int numeroExperimiento*/
    /*Lo que este kernel va a recibir es una fila que deberá devolver ordenada*/    
    /*Este me indica el numero del workitem*/    
    int i = get_global_id (0);     
    int filas = &columnas + 1;    
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
    v[i] = 100;
} ;