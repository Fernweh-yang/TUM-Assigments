#include <stdio.h>
#define N 2

typedef struct 
{
    char Fname[10];
    char Lname[10];
    int ponit_1;
    int point_2;
    int point_3;
    int point_4;
    char Email[20];
}info;

float getGrade(float note){
    if(note>=96.0) return 1.0;
    else if(note>=91) return 1.3;
    else if(note>=86) return 1.7;
    else if(note>=81) return 2.0;
    else if(note>=75.5) return 2.3;
    else if(note>=70.5) return 2.7;
    else if(note>=65.5) return 3.0;
    else if(note>=60.5) return 3.3;
    else if(note>=55.5) return 3.7;
    else if(note>=50.0) return 4.0;
    else return 5.0;
}
int main(){
    
    info p[N]= {{ "Peter","Peter",15,23,15,22,"peter.pasta@tum.de"},
                {"Lisa","Lasagna",22,25,0,12,"lisa.lasagna@tum.de"}};

    /*make a data file*/
    FILE *fp;
    fp = fopen("database.txt","w");
    for(int i=0;i<N;i++){
        fprintf(fp,"%s %s %d %d %d %d %s\n",p[i].Fname,p[i].Lname,p[i].ponit_1,p[i].point_2,p[i].point_3,p[i].point_4,p[i].Email);
    }
    fclose(fp);
    
    /*read data*/
    fopen("database.txt","r");
    info X[N];
    for(int i=0;i<N;i++){
        fscanf(fp,"%s %s %d %d %d %d %s",X[i].Fname,X[i].Lname,&X[i].ponit_1,&X[i].point_2,&X[i].point_3,&X[i].point_4,X[i].Email);
    }
    fclose(fp);

    /*calculate the grade and output*/
    fp=fopen("list.txt","w");
    for(int i=0;i<N;i++){
        float totalNotes = X[i].ponit_1+X[i].point_2+X[i].point_3+X[i].point_4;
        float grade = getGrade(totalNotes);
        fprintf(fp,"%s %s %s: %.1f\n",X[i].Fname,X[i].Lname,X[i].Email,grade);
    }
    fclose(fp);

    return 0;
}