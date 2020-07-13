 #pragma OPENCL EXTENSION cl_intel_printf : enable
__kernel void ordenar(__global int *columnas, __global int *m,__global int *v, __global int *debug, __global int *ne, __global int* fila){
    
    int i = get_global_id (0);        
    int filas = *columnas + 1;       
    v[i] = 0; 
    if(filas>0 && filas<1000000 )
    {                                               
        int inicio =   *columnas*i;                    
        //printf("\nWork item %i\t%i \tInicia en %i",i,filas,inicio);
        //printf("\tColumnas: %i\n",*columnas);              
        for(int x = 0; x <*columnas; x++)
        {            
            //printf("%i\t",m[inicio]);  
            //printf("%i\t",fila[x]);
            fila[x] = m[inicio];            
            inicio++;            
        }		                
                
        for(int p = *columnas / 2; p > 0 ; p = p/2)
        {							
            for(int q = p; q < *columnas; q++)
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
        
                    
        int mediana = fila[*columnas/2];
        //printf("%i) \tmediana\t%i\n",i,mediana);
                
        int indiceV = 0;  			
        for(int f = 0 ; f < (*columnas)*filas; f++)
        {        		
            indiceV= m[f];
            if(mediana == 0)
            {
                indiceV++;
                break;
            }        
            if((indiceV%mediana)==0)
            {
                v[i]++;
            }						                        
        }                
    }
    else
    {
        printf("Error en el work item %i, datos en memoria incorrectos\n",i);
    }

} 