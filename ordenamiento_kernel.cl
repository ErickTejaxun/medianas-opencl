 #pragma OPENCL EXTENSION cl_intel_printf : enable
__kernel void ordenar(__global int *columnas, __global int *m,__global int *v, __global int *debug, __global int *ne, __global int* fila){
    
    int i = get_global_id (0);        
    int filas = *columnas + 1;        
    if(filas ==12)
    {
        printf("Work item %i\t%i\n",i,filas);
        int *inicio = *m + *columnas*i;         
        for(int x = 0; x <*columnas; x++)
        {
            fila[x] = *inicio;
            inicio++;
        }		
        /*
        for(int p = columnas / 2; p > 0 ; p = p/2)
        {							
            for(int q = p; q < columnas; q++)
            {				
                int tmp = fila[q]; 	
                int z = q;			
                for(z = q; z >= p && fila[z-p] > tmp; z = z- p)
                {
                    fila[z] = fila[z - p];
                }
                fila[z] = tmp;				
            }
        }	
        
                    
        int mediana = fila[columnas/2];
                
        int *indiceV = m;  			
        for(int f = 0 ; f < (columnas)*filas; f++)
        {        		
            if(mediana == 0)
            {
                indiceV++;
                break;
            }        
            if((*indiceV%mediana)==0)
            {
                v[i]++;
            }						
            indiceV++;
        }
        v[i] = 100;
        */
    }

} 