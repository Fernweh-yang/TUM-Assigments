# define FILTER_ORDER 3
# include <stdio.h>
# include <stdlib.h>


int main(){
    float co_a[FILTER_ORDER+1] = {1,-2.3812,1.9397,-0.536};
    float co_b[FILTER_ORDER+1] = {0.0028,0.0084,0.0084,0.0028};
    float X[1001] = {0};
    float Y[1001] = {0};

    /*---Read Data---*/
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
    Y[0]=co_b[0]*X[0];
    Y[1]=co_b[0]*X[1] + co_b[1]*X[0] - co_a[1]*Y[0];
    Y[2]=co_b[0]*X[2] + co_b[1]*X[1] + co_b[2]*X[0] - co_a[1]*Y[1] - co_a[2]*Y[0];
    for(int i=3;i<1001;i++){
        Y[i]= co_b[0]*X[i] + co_b[1]*X[i-1] + co_b[2]*X[i-2] + co_b[3]*X[i-3] - co_a[1]*Y[i-1] - co_a[2]*Y[i-2] - co_a[3]*Y[i-3];
    }

    /*---Write Data---*/
    FILE *f;
    f = fopen("Filtered_Signal.txt","w");
    int j=0;
    while(j!=1001){
        fprintf(f,"%f\n",Y[j]);
        j++;
    }
    fclose(f);
}