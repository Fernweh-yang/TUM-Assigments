# define FILTER_ORDER 3
# include <stdio.h>
# include <stdlib.h>


int main(){
    float co_a[FILTER_ORDER+1] = {1,-2.3812,1.9397,-0.536};
    float co_b[FILTER_ORDER+1] = {0.0028,0.0084,0.0084,0.0028};
    float X[1001] = {0};
    float Y_out[1001] = {0};
    float Y_1[1001] = {0};
    float Y_2[1001] = {0};
    float Y_3[1001] = {0};

    /*---Reaf Data---*/
    FILE *fp;
    fp = fopen("signal_combined.txt","r");
    if(fp==NULL){
        perror("The file was not opened\n");
        exit(1);
    }
    int i =0;
    while(i!=1001){
        fscanf(fp,"%f",&X[i]);
        i++;
    }
    fclose(fp);

    /*---Filter---*/
    for(int i=0;i<1001;i++){
        Y_1[i]=co_b[0] * X[i];
        Y_2[i]=Y_1[i]*(1+co_b[1]-co_a[1]);
        Y_3[i]=Y_2[i]*(1+co_b[2]-co_a[2]);
        Y_out[i]=Y_3[i]*(1+co_b[3]-co_a[3]);
        printf("%f\n",Y_out[i]);
    }

    /*---Write Data---*/
    FILE *f;
    f = fopen("Filtered_Signal.txt","w");
    int j=0;
    while(j!=1001){
        fprintf(f,"%f\n",Y_out[i]);
    }
    fclose(f);
}