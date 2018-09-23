#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<limits.h>

#define buff 100
#define delim " \t\n"
//Global variables used in parsing
int regSize=1,fuSize=1,RTopeSize=1,muxSize=1;
int regCount,fuCount,RTopeCount,muxCount,csCount;
//Global variables used in controlflow
int entry=0,count_l,max_timestamp;

//Structure for Register
typedef struct Register{
	char reg[5];
	int width;
}Register;
//Structure for Functional Unit
typedef struct FU{
	char fu[10];
	char oper[100][50];
	int out_wid,in1_wid,in2_wid;
	int no_of_opr;
}FU;

//Structure for read/write operations
typedef struct rwRep{
	char port[5],ioreg[5];
	int io_wid;
}rwRep;

//Structure for assignment opertions
typedef struct assRep{
	char target[5],leftoperand[5],fu[10],fuoper[10],rightoperand[5];
	int target_wid,left_wid,right_wid;
}assRep;
//union for operation representation
typedef union operRep{
	rwRep rwopr;
	assRep assopr;
}operRep;
//structure for RT operations
typedef struct RToperation{
	char block[5],opetype[2];
	operRep oprep;
	int timestamp;
}RToperation;
//structure for Mux
typedef struct MUX{
	char mux[5],**muxinp,muxout[10],**contsig;
}MUX;
typedef struct countsToMinimizemux{
	char reg[5];
	int leftCount,rightCount,countSum;

}countsToMinimizemux;
//structure for control signal
typedef struct CS{
		char blockname[10];
		char *csm;
		char *csr;
		char *csfu;
}CS;

//following structures for control flow
typedef struct block_id_timestamp{
    char block_type;
    int block_id; //id in integer form
    int timestamp;
}b_id_t;

typedef struct bid_count_timestamp{
    char block_type;
    int block_id;
    int *timestamp; 
    int count;  //counting different time stamp
}t_count;

typedef struct cfile{
int choice;char bol;
char source_block,dest_block1,dest_block2;
int  source_block_id,dest_block1_id,dest_block2_id;
}cfile;

//function declarations
Register *parseReg(FILE *,int);						//parsing Registers from file
FU *parseFu(FILE *,int);							//parsing Fu's from file			
RToperation *parseRToper(FILE *,int);				//parsing RToperations from file
void minimizeMux(Register *,FU *,RToperation *);	//minimizes number of MUX required
MUX *storingMux(Register *,FU *,RToperation *);		//generate/store Muxes
int CS_for_FU(FU *);								//Control signals required for Fu's
int CS_for_Registers(Register *);					//Control signals required for Registers	
void readmux(MUX *);
int get_csm_length(MUX *);							
CS *CS_for_Blocks(RToperation *, int,int,int,MUX *,FU *);		//generates Control signals	
b_id_t* get_block_id_timestamp(RToperation *rtList);			
t_count* get_t_count(b_id_t* block_list);
int c1(char block_type,int block_id,t_count *ctime,int entry);
int c2(int time_stamp,t_count* ctime,int check1);
void generate_control_flow(RToperation* rtList,FILE *outfp);	//generates Control flow
void FileWrite(CS *csList, Register *,FU *,RToperation *,MUX *);//writing Output into file 
int line_to_int(char *l);						
char* removeSpace(char *);
void printRTList(RToperation *);
void printMuxList(MUX *);
char * decTobin(int ,int );
int logbase2(unsigned long long);
void itos(int num, char *str);

int main(int argc, char const *argv[])
{
	if(!argv[1]){
		printf("Enter argument as path of inputfile\n");
		return 0;
	}
	FILE *inp=fopen(argv[1],"r");
	if(!inp){
		perror("fopen");
		return 0;
	}
	RToperation *rtList=parseRToper(inp,16);
	Register *regList=parseReg(inp,9); //file pointer & initial reg count
	FU *fuList=parseFu(inp,4);
	fclose(inp);
	printf("After Parsing RToperation\n");
	printRTList(rtList);
	minimizeMux(regList,fuList,rtList);
	MUX *muxList=storingMux(regList,fuList,rtList);
	printf("\n\nAfter Minizing Mux\n\n");
	printRTList(rtList);
	printMuxList(muxList);
	int fu_cs_length = CS_for_FU(fuList);
	int no_of_reg = CS_for_Registers(regList);
	int mux_cs_length=get_csm_length(muxList);
	//printf("&&&&&&&&&&&&&&&&&&&&&&&&&&& %d\n",mux_cs_length);
	CS *csList= CS_for_Blocks(rtList,mux_cs_length,no_of_reg,fu_cs_length,muxList,fuList);
	//readmux(muxList);
	FileWrite(csList,regList,fuList,rtList,muxList);
}

void itos(int num, char *str)
{
        if(str == NULL)
        {
                return;
        }
        sprintf(str, "%d", num);
}

int line_to_int(char *l)
{
 int j,k,nchar,id; ssize_t size; char ch; size=strlen(l);k=size;id=0;
 for(j=0;j<size;j++) { ch=l[j]; nchar=ch-'0';if(nchar>=0&&nchar<10) id=id*10+nchar; }
 return id;  	    
}

void generate_control_flow(RToperation *rtList,FILE *outfp)
{   
	int i;
	//Finding maximum timestamp
	for(i=0;i<RTopeCount;i++)
		if(max_timestamp<rtList[i].timestamp)
			max_timestamp=rtList[i].timestamp;
	max_timestamp++;
    b_id_t* block_list=get_block_id_timestamp(rtList);
	t_count* ctime=get_t_count(block_list);
	
    printf("\n******Output of ctime -- (block, id, #timestamps)\n");
	for(i=0;i<entry;i++)	{  printf("%c",ctime[i].block_type);printf(" %d",ctime[i].block_id);printf("  %d\n",ctime[i].count);}
	printf("\n******Control Flow\n\n");
  
    FILE *fp; char line[2*buff]; int count_l=0; char *l;

    fp=fopen("Flow_Control.txt","r");
    while(fgets(line,2*buff,fp)){count_l++;}fclose(fp);
    //count_l--;
    cfile* parse=(cfile*)malloc(count_l*sizeof(cfile));
    fp=fopen("Flow_Control.txt","r");

     i=0;int j,k,id,nchar;ssize_t size;  int s_count; char ch;
    //fgets(line,2*buff,fp);
    while(fgets(line,2*buff,fp))
    {//printf("enterd\n");
      s_count=0;
      l=strtok(line,delim);
      while(l!=NULL)
      {
        if(s_count==0){parse[i].source_block=l[0];  id=line_to_int(l);  parse[i].source_block_id=id;}
      
        if(s_count==1){id=line_to_int(l);    parse[i].choice=id;}
         
        if(s_count==2 && parse[i].choice==1){
            parse[i].dest_block1=l[0];    id=line_to_int(l);   parse[i].dest_block1_id=id;         break;}
        
        if(s_count==2 && parse[i].choice==2){
          parse[i].bol=l[0];l=strtok(NULL,delim);parse[i].dest_block1=l[0];id=line_to_int(l); parse[i].dest_block1_id=id;l=strtok(NULL,delim);       
          l=strtok(NULL,delim); parse[i].dest_block2=l[0]; id=line_to_int(l);parse[i].dest_block2_id=id; }
        
         if(s_count==2 && parse[i].choice==0)break;
         
        l=strtok(NULL,delim);
        s_count++;   
      }      
     i++;
    }fclose(fp);

  //fp=fopen("control_flow_output.txt","w");
    printf("cout %d\n",count_l );
  fprintf(outfp,"\n<Controller FSM>\n");
  int v1icount;char *prnt=malloc(2*buff*sizeof(char));
  for(i=0;i<count_l;i++)
  {

    for(k=0;k<entry;k++)
    {
       if((ctime[i].block_type==parse[i].source_block)&&(ctime[i].block_id==parse[i].source_block_id))
       {
        v1icount=ctime[i].count;
       }
    }
    //printf("entered into for\n");
    if(parse[i].source_block=='B'){
        for(j=0;j<v1icount;j++){
	        if(j==v1icount-1){
	            if(parse[i].choice==0){
	                sprintf(prnt,"%c%d%d 1 %c%d%d\n",parse[i].source_block,parse[i].source_block_id,j+1,parse[i].source_block,parse[i].source_block_id,j+2); 
	                printf("%s\n",prnt); 
	                fputs(prnt,outfp);
	                sprintf(prnt,"%c%d%d %d\n",parse[i].source_block,parse[i].source_block_id,j+2,parse[i].choice); 
	                printf("%s\n",prnt);
	                fputs(prnt,outfp);
	            }
	            else{
	            	sprintf(prnt,"%c%d%d %d %c%d1\n",parse[i].source_block,parse[i].source_block_id,j+1,parse[i].choice,parse[i].dest_block1,parse[i].dest_block1_id); 
	            	printf("%s\n",prnt); fputs(prnt,outfp);
	    		}
	     
	        } 
	        else
	        {
				if(parse[i].choice==0){
				sprintf(prnt,"%c%d%d 1 %c%d%d\n",parse[i].source_block,parse[i].source_block_id,j+1,parse[i].source_block,parse[i].source_block_id,j+2); 
				printf("%s\n",prnt); fputs(prnt,outfp);
				}
				else{     
				sprintf(prnt,"%c%d%d %d %c%d%d\n",parse[i].source_block,parse[i].source_block_id,j+1,parse[i].choice,parse[i].source_block,parse[i].source_block_id,j+2);
				printf("%s\n",prnt);fputs(prnt,outfp);
				}      
	        }
	    }       
    }
	if(parse[i].source_block=='C')
	{
		char ch1,ch2;
		if(parse[i].bol=='T'){ch1='T';ch2='F';}
		if(parse[i].bol=='F'){ch1='F';ch2='T';}
		sprintf(prnt,"%c%d1 %d %c %c%d1 %c %c%d1\n",parse[i].source_block,parse[i].source_block_id,parse[i].choice,ch1,parse[i].dest_block1,parse[i].dest_block1_id,ch2,parse[i].dest_block2,parse[i].dest_block2_id);
		printf("%s\n",prnt);fputs(prnt,outfp);
	}
  }
  return;
}

int c2(int time_stamp,t_count* ctime,int check1)
{
  int i;
  for(i=0;i<max_timestamp;i++)
  {
   if(ctime[check1].timestamp[i]==time_stamp)
   return i;
  }
  return -1;
}

int c1(char block_type,int block_id,t_count *ctime,int entry)
{
    int i;
    if(entry==0)
    return -1;
    
    for(i=0;i<entry;i++)
    {
          if(ctime[i].block_type==block_type && ctime[i].block_id==block_id)
          return i; 
    }
    return -1;
}

t_count* get_t_count(b_id_t* block_list)
{
    
    t_count* ctime=NULL; 
    
    int check1=1; int check2=-1; entry=0;
    
    int i,j,k;
    
    for(i=0;i<RTopeCount;i++)
    {
        check1=c1(block_list[i].block_type,block_list[i].block_id,ctime,entry);
        if(entry==0)
        {
            entry++;
            ctime=(t_count*)malloc(entry*sizeof(t_count));
            k=entry-1;
            ctime[k].timestamp=(int*)calloc(max_timestamp,sizeof(int));
            ctime[k].block_type=block_list[i].block_type;
            ctime[k].block_id=block_list[i].block_id;
            ctime[k].count=1;
            ctime[k].timestamp[0]=block_list[i].timestamp;
            for(j=1;j<max_timestamp;j++)
            	ctime[k].timestamp[j]=-1;
         }
        else
        {
         if(check1==-1)
         {
            entry++;
            ctime=(t_count*)realloc(ctime,entry*sizeof(t_count));
            k=entry-1;
            ctime[k].timestamp=(int*)calloc(max_timestamp,sizeof(int));
            ctime[k].block_type=block_list[i].block_type;
            ctime[k].block_id=block_list[i].block_id;
            ctime[k].count=1;
            ctime[k].timestamp[0]=block_list[i].timestamp;
            for(j=1;j<max_timestamp;j++)
            ctime[k].timestamp[j]=-1;
            
         }
         else
         {
           check2=c2(block_list[i].timestamp,ctime,check1);
           if(check2==-1)
           {
            j=0;
            while( ctime[check1].timestamp[j]!=-1)
            j++;
            ctime[check1].timestamp[j]=block_list[i].timestamp;
            ctime[check1].count=ctime[check1].count+1;
           }
        }
        }
     }
     return ctime; 
}

b_id_t* get_block_id_timestamp(RToperation *rtList)
{
    b_id_t* block_info=(b_id_t*)malloc(RTopeCount*sizeof(b_id_t));
    int i;int j,k,id; char ch;
    for(i=0;i<RTopeCount;i++)
    {
		block_info[i].block_type=rtList[i].block[0];
		id=0;
		for(j=1;j<5;j++)
		{
			ch=rtList[i].block[j];
			k=ch-'0';
			if(k>=0&&k<10)
				id=id*10+k;
		}
		block_info[i].block_id=id;
		block_info[i].timestamp=rtList[i].timestamp;
    }
    return(block_info);
}

void FileWrite(CS *csList, Register *regList,FU *fuList,RToperation *rtList,MUX *muxList)
{
	int i=0,j=0,check,checkInd;
	FILE *fp;
	fp = fopen ("DataPath_Controller_Ouput.txt", "w");
	fprintf(fp,"#Data path\n");
	fprintf(fp,"<Specify Registers>\n");
	while(i<regCount)
	{
		check=0;
		for(checkInd=0;checkInd<RTopeCount;checkInd++)
			if(!strcmp(regList[i].reg,rtList[checkInd].oprep.assopr.target) || !strcmp(regList[i].reg,rtList[checkInd].oprep.assopr.leftoperand) || !strcmp(regList[i].reg,rtList[checkInd].oprep.assopr.rightoperand))
				check=1;
		if(check)
			fprintf(fp,"%s, %d\n",regList[i].reg,regList[i].width);
		i++;
	}
	i=0;
	fprintf(fp,"\n<Specify FUs>\n");
	while(i<fuCount)
	{
		check=0;
		for(checkInd=0;checkInd<RTopeCount;checkInd++)
			if(!strcmp(fuList[i].fu,rtList[checkInd].oprep.assopr.fu))
				check=1;
		if(check){
			fprintf(fp,"%s, (",fuList[i].fu);
			j=0;
		   	while(strcmp(fuList[i].oper[++j],"NULL"))
		   		fprintf(fp, "%s,", fuList[i].oper[j-1]);
		   	fprintf(fp, "%s", fuList[i].oper[j-1]);
		   	fprintf(fp,"), (%d,%d,%d)\n",fuList[i].out_wid,fuList[i].in1_wid,fuList[i].in2_wid);
		}
		i++;
	}
	i=0;
	fprintf(fp,"\n<Specify RT operations> \n");
	while(i<RTopeCount)
	{
     	if(!strcmp(rtList[i].opetype,"R") || !strcmp(rtList[i].opetype,"O")){
			fprintf(fp,"%s, %d, %s, ",rtList[i].block,rtList[i].timestamp,rtList[i].opetype);
  	        fprintf(fp,"(%s, %s, %d)\n",rtList[i].oprep.rwopr.port,rtList[i].oprep.rwopr.ioreg,rtList[i].oprep.rwopr.io_wid);
         }
		else{
		    fprintf(fp,"%s, %d, %s, ",rtList[i].block,rtList[i].timestamp,rtList[i].opetype);
		    fprintf(fp,"(%s, %d), ",rtList[i].oprep.assopr.target, rtList[i].oprep.assopr.target_wid);
			fprintf(fp,"(%s, %d), ",rtList[i].oprep.assopr.leftoperand, rtList[i].oprep.assopr.left_wid);
			fprintf(fp,"(%s, %s), ",rtList[i].oprep.assopr.fu,rtList[i].oprep.assopr.fuoper);
	 		fprintf(fp,"(%s, %d)\n",rtList[i].oprep.assopr.rightoperand,rtList[i].oprep.assopr.right_wid);
	    }
	   i++;
	}
	i=0;
	fprintf(fp,"\n<Specify MUX> \n");
	//fprintf(fp,"<MUX Name> <Input of MUX>, <Output of MUX>, <Control signal in order> \n");
	while(i<muxCount)
	{
		fprintf(fp,"<%s><", muxList[i].mux);
		for(j=0;strcmp(muxList[i].muxinp[j],"NULL");j++)
		{
			if(j==0)
			fprintf(fp,"%s",muxList[i].muxinp[j]);
			else
			fprintf(fp,",%s",muxList[i].muxinp[j]);
		}
		fprintf(fp,"><%s><",muxList[i].muxout);
		for(j=0;muxList[i].contsig[j];j++)
		{
			if(j==0)
			fprintf(fp,"%s",muxList[i].contsig[j]);
			else
			fprintf(fp,",%s",muxList[i].contsig[j]);
		}
		fprintf(fp,"> \n");
		i++;
	}
	//** Control flow generation and writing into file
	generate_control_flow(rtList,fp);

	fprintf(fp,"\n<CS for MUX in order>\n");
	//fprintf(fp,"(#MUX, #Control bits):\n");
	i=0;
	while(i<muxCount)
	{
		fprintf(fp,"(%s", muxList[i].mux);
		fprintf(fp,",%d)",(int)strlen(muxList[i].contsig[0]));

		i++;
		if(i<muxCount)
			fprintf(fp, ", ");
	}
	fprintf(fp, "\n");

	i=0;
	fprintf(fp,"\n<CS for Reg enable in order>\n");
	fprintf(fp,"(");
	while(i<regCount)
	{
		fprintf(fp,"1");
		i++;
		if(i<regCount)
			fprintf(fp, ", ");
	}
	fprintf(fp, ")\n");


	i=0;
	fprintf(fp,"\n<CS for FU in order>\n");
	while(i<fuCount)
	{
		fprintf(fp,"(%s,",fuList[i].fu);
		fprintf(fp, "%d)",logbase2(fuList[i].no_of_opr));
	    i++;
	    if(i<fuCount)
			fprintf(fp, ", ");
	}
	fprintf(fp, "\n");

	i=0;
	fprintf(fp,"\n<CAP>\n");
	//fprintf(fp,"\n(CS for MUX) (CS for Reg en) (CS for FU)\n");
	//printf("%d\n",csCount);
	while(i<csCount)
	{
		fprintf(fp,"%s:(%s)(%s)(%s)\n",csList[i].blockname,csList[i].csm,csList[i].csr,csList[i].csfu);
		i++;
	}
	fprintf(fp, "\n");




	fclose(fp);
}

int logbase2(unsigned long long x)
{
  static const unsigned long long t[6] = {
    0xFFFFFFFF00000000ull,
    0x00000000FFFF0000ull,
    0x000000000000FF00ull,
    0x00000000000000F0ull,
    0x000000000000000Cull,
    0x0000000000000002ull
  };

  int y = (((x & (x - 1)) == 0) ? 0 : 1);
  int j = 32;
  int i;

  for (i = 0; i < 6; i++) {
    int k = (((x & t[i]) == 0) ? 0 : j);
    y += k;
    x >>= k;
    j >>= 1;
  }

  return y;
}




int CS_for_FU(FU *fuList){

	int i=-1;
	//printf("\n\nHere we Go->\n");
	printf("\n\nCS for FU in order:\n\n(FU, #Control bits)\n\n");
	//int n=sizeof(regList);
	//printf("size is %d\n",n);
	int max=0;
	printf("check: above while\n" );
	while(strcmp(fuList[++i].fu,"NULL")!=0){
		printf("below while\n");
		if(i!=0)
			printf(", ");
		//printf("%s\n",fuList[i].fu);
		int opCou=-1;
		int cnt=0;
		while(strcmp(fuList[i].oper[++opCou],"NULL")!=0){
			//printf("%s\n",fuList[i].oper[opCou]);
			cnt++;

		}
		printf("second while\n");
		fuList[i].no_of_opr=cnt;
		printf("(%s, %d)",fuList[i].fu,logbase2(fuList[i].no_of_opr=cnt));
		max+=logbase2(cnt);
		//(ADDDER, 1)
	}

	return max;
}


int CS_for_Registers(Register *regList){

	int i=-1;
	//printf("\n\nHere we Go->\n");
	printf("\n\nCS for Reg enable in order:\n\n");
	//int n=sizeof(regList);
	//printf("size is %d\n",n);
	printf("(");
	while(strcmp(regList[++i].reg,"NULL")!=0){
		if(i!=0)
			printf(",");
		printf("1");
	}
	printf(")");

	return i;
}



void readmux(MUX *muxList){
	int i=-1;
	while(strcmp(muxList[++i].mux,"NULL")!=0){
		printf("print %s\n",muxList[i].mux );
		printf("print now %s\n",muxList[i].muxinp[0]);
		printf("print now %s\n",muxList[i].muxinp[1]);
		printf("print now %s\n",muxList[i].muxinp[2]);
	}
}

int get_csm_length(MUX *muxList){
	int ind=-1;
	int z=0;
	while(strcmp(muxList[++ind].mux,"NULL")!=0 ){
		z+=strlen(muxList[ind].contsig[0]);
	}
	return z;
}

CS *CS_for_Blocks(RToperation *rtList, int mux_cs_length,int no_of_reg,int fu_cs_length,MUX *muxList,FU *fuList){

	//printf("\nsomething new is here muxCount %d\n",muxCount);



	printf("\n");
	int CS_count=2;

	CS *signals =malloc(sizeof(CS)*CS_count);

	int k=0;
	int i=-1;

	//allocating memory for length of mux_signal, reg_signal and fu_signal
	signals[k].csm=(char*)calloc(mux_cs_length+1,sizeof(char));
	signals[k].csr=(char*)calloc(no_of_reg+1,sizeof(char));
	signals[k].csfu=(char*)calloc(fu_cs_length+1,sizeof(char));
	strcpy(signals[0].blockname,"NULL");
	//printf("after signals\n");
	int cnt=1;

	//reading block one by one
	while(strcmp(rtList[++i].block,"NULL")!=0){
		char temp[10];
		sprintf(temp,"%s%d",rtList[i].block,rtList[i].timestamp);

		//if block is read or write mode
		if(strcmp(rtList[i].opetype,"R")==0 || strcmp(rtList[i].opetype,"O")==0 ){
			//printf("\n\nread mode:\n");

			int j=-1;
			int found=0;	// to verify this reading this (block + timestamp) first time


			//checking if this block has already some signals, then update
			while(strcmp(signals[++j].blockname,"NULL")!=0){

				if(strcmp(signals[j].blockname,temp)==0){
					int x=atoi(rtList[i].oprep.rwopr.ioreg+1);
					signals[j].csr[x]='1';
					//printf("block is %s\n",signals[j].blockname);
					//printf("again csr in read mode\n");
					//printf("changed as position %d\n",x);
					found=1;
					break;
				}
				//++j;
			}

			//first time reading the current block with timestamp
			if(found==0){

				//printf("first time in read mode\n");
				strcpy(signals[k].blockname,temp);
				//printf("block is %s\n",signals[k].blockname);

				//initializing with 0's
				int x=0;
				for(x=0; x<mux_cs_length; x++)
					signals[k].csm[x]='0';
				signals[k].csm[x]='\0';
				//printf("signals[k].csm %s\n",signals[k].csm);
				for(x=0; x<no_of_reg; x++)
					signals[k].csr[x]='0';
				signals[k].csr[x]='\0';
				//printf("signals[k].csr %s\n",signals[k].csr);
				for(x=0; x<fu_cs_length; x++)
					signals[k].csfu[x]='0';
				signals[k].csfu[x]='\0';
				//printf("signals[k].csfu %s\n",signals[k].csfu);

				x=atoi(rtList[i].oprep.rwopr.ioreg+1);
				//printf("fist x is %d\n",x);

				//activating the register signal
				signals[k].csr[x]='1';
				//printf("changed at position %d\n",x);
				k++;

				//memory allocation if required
				if((k+1)>=CS_count){
					CS_count*=2;
					signals=realloc(signals,sizeof(CS)*CS_count);
				}

				//mermory allocation for next signals.
				signals[k].csm=(char*)calloc(mux_cs_length+1,sizeof(char));
				signals[k].csr=(char*)calloc(no_of_reg+1,sizeof(char));
				signals[k].csfu=(char*)calloc(fu_cs_length+1,sizeof(char));
				strcpy(signals[k].blockname,"NULL");
			}

		}

		//if block is in assignemnt mode
		else if(strcmp(rtList[i].opetype,"A")==0){
			//printf("\n\nassignment mode\n");

			int j=-1;
			int found=0;// to verify this reading this (block + timestamp) first time


			//checking if this block has already some signals, then update
			while(strcmp(signals[++j].blockname,"NULL")!=0){
				//printf("inside while\n");
				if(strcmp(signals[j].blockname,temp)==0){

					//***** get_muxcs() ******
					int ind=-1;		//will change after corrction of M1
					int z=0;	//will change after corrction of M1
					//printf("\nfor block %s\n",signals[k].blockname);
					while(strcmp(muxList[++ind].mux,"NULL")!=0 ){
						//printf("comparing passed with %s and %s\n",muxList[ind].muxout,rtList[i].oprep.assopr.fu);
						int l=-1;
						//printf("this is %s\n",muxList[ind].mux);
						while(strcmp(muxList[ind].muxinp[++l],"NULL")!=0 && strcmp(muxList[ind].muxout,rtList[i].oprep.assopr.fu)==0){
							//printf("yahoo l is %d\n",l);
							//printf("now %s\n",muxList[ind].muxinp[l]);
							if(strcmp(muxList[ind].muxinp[l],rtList[i].oprep.assopr.leftoperand)==0){

								int mux_sig=strlen(muxList[ind].contsig[0]);
								int m;
								for(m=z; m<mux_sig+z; m++){
									char tem=(char)muxList[ind].contsig[l][m-z];
									signals[j].csm[m]=tem;
								}
								//printf("\n%s",rtList[i].oprep.assopr.leftoperand);
							}
							//printf("second if\n");
							//printf("temp is %s, block is %s, blockname %s\n",temp,rtList[i].block,signals[j].blockname);
							if(strcmp(muxList[ind].muxinp[l],rtList[i].oprep.assopr.rightoperand)==0){
								//printf("first if\n");
								int mux_sig=strlen(muxList[ind].contsig[0]);
								int m;
								//printf("\nz is %d and mux_sig is %d\n",z,mux_sig);
								//printf("now csm %s\n",signals[j].csm);
								for(m=z; m<mux_sig+z; m++){
									char tem=(char)muxList[ind].contsig[l][m-z];
									signals[j].csm[m]=tem;
								}
								//printf("\n%s",rtList[i].oprep.assopr.rightoperand);
								//printf("okok\n");
							}
							//printf("yaha tk ok h\n");
						}
						z+=strlen(muxList[ind].contsig[0]);
					}




					//*****CS of FU ********
					ind=-1;
					z=0;
					while(strcmp(fuList[++ind].fu,"NULL")!=0){
						int l=0;
						if(strcmp(fuList[ind].fu,rtList[i].oprep.assopr.fu)==0){
							l=fuList[ind].no_of_opr;
							if(l>1){
								int m=0;
								for(m=0; m<l; m++){
									if(strcmp(fuList[ind].oper[m],rtList[i].oprep.assopr.fuoper)==0){
										int n=logbase2(l);
										char *str=calloc(n+1,sizeof(char));
										str=decTobin(m,n);
										int y;
										for(y=z; y<n+z; y++){
											signals[j].csfu[y]=str[y-z];
										}
										break;
									}
								}
							}
						}
						z+=logbase2(fuList[ind].no_of_opr);
					}


					//******* CS for Reg **********

					//activating register signals
					int x=atoi(rtList[i].oprep.assopr.target+1);
					signals[j].csr[x]='1';
					//printf("again csr %s %s\n",signals[j].blockname,rtList[i].oprep.assopr.target);
					found=1;
					break;
				}
				//++j;
			}


			if(found==0){
				//printf("first impression cnt : %d\n",cnt++);
			//otherwise
				//printf("\nfirst time\n");
				strcpy(signals[k].blockname,temp);
				//printf("block is %s\n",signals[k].blockname);


				//initializing with 0's
				int x=0;
				for(x=0; x<mux_cs_length; x++)
					signals[k].csm[x]='0';
				signals[k].csm[x]='\0';
				//printf("signals[k].csm %s\n",signals[k].csm);
				for(x=0; x<no_of_reg; x++)
					signals[k].csr[x]='0';
				signals[k].csr[x]='\0';
				//printf("signals[k].csr %s\n",signals[k].csr);
				for(x=0; x<fu_cs_length; x++)
					signals[k].csfu[x]='0';
				signals[k].csfu[x]='\0';
				//printf("signals[k].csfu %s\n",signals[k].csfu);


				//***** get_muxcs() ******
				j=-1;		//will change after corrction of M1
				int z=0;	//will change after corrction of M1
				//printf("\nfor block %s\n",signals[k].blockname);
				while(strcmp(muxList[++j].mux,"NULL")!=0){
					int l=-1;
					//printf("this is %s\n",muxList[j].mux);
					while(strcmp(muxList[j].muxinp[++l],"NULL")!=0 && strcmp(muxList[j].muxout,rtList[i].oprep.assopr.fu)==0){
						//printf("now %s\n",muxList[j].muxinp[l]);
						//printf("inside while: signals[k].csm %s\n",signals[k].csm);
						if(strcmp(muxList[j].muxinp[l],rtList[i].oprep.assopr.leftoperand)==0){
							int mux_sig=strlen(muxList[j].contsig[0]);
							int m;
							for(m=z; m<mux_sig+z; m++){
								char tem=(char)muxList[j].contsig[l][m-z];
								signals[k].csm[m]=tem;
							}
							//printf("\ncsm%s",signals[k].csm);
							//printf("\n%s\n",rtList[i].oprep.assopr.leftoperand);
						}
						//printf("second if\n");
						if(strcmp(muxList[j].muxinp[l],rtList[i].oprep.assopr.rightoperand)==0){
							int mux_sig=strlen(muxList[j].contsig[0]);
							int m;
							for(m=z; m<mux_sig+z; m++){
								char tem=(char)muxList[j].contsig[l][m-z];
								signals[k].csm[m]=tem;
							}
							//printf("\n%s",rtList[i].oprep.assopr.rightoperand);
						}
					}
					z+=strlen(muxList[j].contsig[0]);
				}

				//printf("\ncsm signal is %s\n",signals[k].csm);


				//*******CS for Reg********
				x=atoi(rtList[i].oprep.assopr.target+1);
				signals[k].csr[x]='1';



				//*****CS of FU ********
				j=-1;
				z=0;
				while(strcmp(fuList[++j].fu,"NULL")!=0){
					int l=0;
					//printf("\n\n\n%s\n",fuList[j].fu,rtList[i]);
					//printf("value of z is %d\n",z);
					if(strcmp(fuList[j].fu,rtList[i].oprep.assopr.fu)==0){
						l=fuList[j].no_of_opr;
						if(l>1){
							int m=0;
							for(m=0; m<l; m++){
								if(strcmp(fuList[j].oper[m],rtList[i].oprep.assopr.fuoper)==0){
									int n=logbase2(l);
									char *str=calloc(n+1,sizeof(char));
									str=decTobin(m,n);
									int y;
									//printf("************* %s\n",signals[k].csfu);
									for(y=z; y<n+z; y++){
										signals[k].csfu[y]=str[y-z];
										//printf("*************value of y %d signal %s\n",y,str);
										//printf("************* %s\n",signals[k].csfu);
									}

									break;
								}
							}
						}
					}
					z+=logbase2(fuList[j].no_of_opr);
				}



				k++;

				//allocation of memory if required
				if((k+1)>=CS_count){
					CS_count*=2;
					signals=realloc(signals,sizeof(CS)*CS_count);
				}

				//mermory allocation for next signals.
				signals[k].csm=(char*)calloc(mux_cs_length+1,sizeof(char));
				signals[k].csr=(char*)calloc(no_of_reg+1,sizeof(char));
				signals[k].csfu=(char*)calloc(fu_cs_length+1,sizeof(char));
				strcpy(signals[k].blockname,"NULL");
			}


		}




		int j=-1;
		while(strcmp(rtList[++j].block,"NULL")!=0){

			if(strcmp(rtList[i].oprep.assopr.target,rtList[j].oprep.assopr.target)==0){
				//printf("some\n");
				if(strcmp(rtList[i].oprep.assopr.fu,rtList[j].oprep.assopr.fu)==0){
					//printf("yes\n");
					continue;
					//printf("no\n");
				}
				else{
					int ind=-1;
					int z=0;
					while(strcmp(muxList[++ind].mux,"NULL")!=0){

						//printf("this is %s\n",muxList[ind].mux);
						if(strcmp(muxList[ind].muxout,rtList[i].oprep.assopr.target)==0){
							//printf("uska output apna target %s %s\n",muxList[ind].muxout,rtList[i].oprep.assopr.target);
							int l=-1;
							while(strcmp(muxList[ind].muxinp[++l],"NULL")!=0){
								//printf("yaha tk aa gaya\n");
								//printf("fus %s %s\n",muxList[ind].muxinp[l],rtList[i].oprep.assopr.fu);
								if(strcmp(muxList[ind].muxinp[l],rtList[i].oprep.assopr.fu)==0){
									//printf("pahuch gaya\n");
									int m=strlen(muxList[ind].contsig[0]);
									char *str=calloc(m+1,sizeof(char));
									str=decTobin(l,m);
									int y;
									int blk=-1;
									while(strcmp(signals[++blk].blockname,temp)!=0);
									for(y=z; y<m+z; y++){
											signals[blk].csm[y]=str[y-z];
									}
									//printf("**********************%s\n",signals[k-1].csm);
									break;
								}
							}
						}
						z+=strlen(muxList[ind].contsig[0]);

					}
				}
			}
			//printf("end\n");
		}


	}
	csCount=k;
	k=-1;
	while(strcmp(signals[++k].blockname,"NULL")!=0){
		printf("\n%s %s %s %s\n",signals[k].blockname,signals[k].csm,signals[k].csr,signals[k].csfu);
	}

	return signals;
}

char *bin;
char * decTobin(int n,int bit){
	bin=(char*)calloc(bit+1,sizeof(char));
	bin[bit]='\0';
	while(bit--){
		if(n>0){
			bin[bit]=n%2+'0';
			n/=2;
		}
		else
			bin[bit]='0';
	}
	return bin;
}
void printMuxList(MUX *list){
	int i,j;
	printf("\nMux List\n\n");
	for(i=0;i<muxCount;i++){
		printf("%s ",list[i].mux);
		for(j=0;strcmp(list[i].muxinp[j],"NULL");j++)
			printf("%s,",list[i].muxinp[j]);
		printf(" %s ",list[i].muxout);
		for(j=0;list[i].contsig[j];j++)
			printf("%s,",list[i].contsig[j]);
		printf("\n");
	}
}
MUX *storingMux(Register *regList,FU *fuList,RToperation *rtList){
	//printf("storing mux\n");
	int i,j,k,l,uniqleft[regCount],uniqright[regCount],uniqleftCount,uniqrightCount,muxInd=0;
	MUX *list=(MUX *)calloc(muxSize,sizeof(MUX));
	char buf[5]={};
	for(i=0;i<fuCount;i++){
		for(k=0;k<regCount;k++){
				uniqleft[k]=0;
				uniqright[k]=0;
		}
		uniqleftCount=0;uniqrightCount=0;
		for(j=0;j<RTopeCount;j++){
			if(!strcmp(rtList[j].opetype,"A") && !strcmp(fuList[i].fu,rtList[j].oprep.assopr.fu)){
				for(l=0;l<regCount;l++){
					if(!strcmp(rtList[j].oprep.assopr.leftoperand,regList[l].reg)){
						uniqleft[l]=1;
					}
					if(!strcmp(rtList[j].oprep.assopr.rightoperand,regList[l].reg)){
						uniqright[l]=1;
					}
				}
			}
		}
		for(k=0;k<regCount;k++){
			uniqleftCount+=uniqleft[k];
			uniqrightCount+=uniqright[k];
		}
		if(muxInd>=muxSize){
			muxSize*=2;
			list=realloc(list,sizeof(MUX)*muxSize);
		}
		if(uniqleftCount>1){
			//printf("left\n");
			itos(muxInd+1,buf+1);
			buf[0]='M';
			strcpy(list[muxInd].mux,buf);
			//printf("%s ",list[muxInd].mux);
			list[muxInd].muxinp=(char**)calloc(uniqleftCount+1,sizeof(char*));
			for(k=0,l=0;k<regCount;k++){
				if(uniqleft[k]){
					list[muxInd].muxinp[l]=(char*)calloc(5,sizeof(char));
					strcpy(list[muxInd].muxinp[l],regList[k].reg);
					//printf("%s,",list[muxInd].muxinp[l] );
					l++;
				}
			}
			list[muxInd].muxinp[l]=(char*)calloc(5,sizeof(char));
			strcpy(list[muxInd].muxinp[l],"NULL");

			strcpy(list[muxInd].muxout,fuList[i].fu);
			//printf(" %s ",list[muxInd].muxout );
			list[muxInd].contsig=(char**)calloc(uniqleftCount+1,sizeof(char*));
			for(l=0;l<uniqleftCount;l++){

					list[muxInd].contsig[l]=(char*)calloc((int)ceil(log(uniqleftCount)/log(2))+1,sizeof(char));
					strcpy(list[muxInd].contsig[l],decTobin(l,(int)ceil(log(uniqleftCount)/log(2))));
					//printf("%s,",list[muxInd].contsig[l] );
			}
			//printf("\n");
			muxInd++;
		}
		if(muxInd>=muxSize){
			muxSize*=2;
			list=realloc(list,sizeof(MUX)*muxSize);
		}
		if(uniqrightCount>1){
			//printf("right\n");
			itos(muxInd+1,buf+1);
			buf[0]='M';
			strcpy(list[muxInd].mux,buf);
			//printf("%s ",list[muxInd].mux);
			list[muxInd].muxinp=(char**)calloc(uniqrightCount+1,sizeof(char*));
			for(k=0,l=0;k<regCount;k++){
				if(uniqright[k]){
					list[muxInd].muxinp[l]=(char*)calloc(5,sizeof(char));
					strcpy(list[muxInd].muxinp[l],regList[k].reg);
					//printf("%s,", list[muxInd].muxinp[l]);
					l++;
				}

			}
			list[muxInd].muxinp[l]=(char*)calloc(5,sizeof(char));
			strcpy(list[muxInd].muxinp[l],"NULL");

			strcpy(list[muxInd].muxout,fuList[i].fu);
			//printf(" %s ",list[muxInd].muxout );
			list[muxInd].contsig=(char**)calloc(uniqrightCount+1,sizeof(char*));
			for(l=0;l<uniqrightCount;l++){
					list[muxInd].contsig[l]=(char*)calloc((int)ceil(log(uniqrightCount)/log(2))+1,sizeof(char));
					strcpy(list[muxInd].contsig[l],decTobin(l,(int)ceil(log(uniqrightCount)/log(2))));
					//printf("%s,",list[muxInd].contsig[l]);
			}
			//printf("\n");
			muxInd++;
		}
	}
	//mux as input reg as output
	int uniqfu[fuCount],uniqfuCount;
	for(i=0;i<regCount;i++){
		if(muxInd>=muxSize){
			muxSize*=2;
			list=realloc(list,sizeof(MUX)*muxSize);
		}
		for(k=0;k<fuCount;k++)
			uniqfu[k]=0;
		uniqfuCount=0;
		for(j=0;j<RTopeCount;j++){
			if(!strcmp(rtList[j].oprep.assopr.target,regList[i].reg)){
				for(k=0;k<fuCount;k++){
					if(!strcmp(fuList[k].fu,rtList[j].oprep.assopr.fu))
						uniqfu[k]=1;
				}
			}
		}
		for(k=0;k<fuCount;k++)
			uniqfuCount+=uniqfu[k];
		if(uniqfuCount>1){
			itos(muxInd+1,buf+1);
			buf[0]='M';
			strcpy(list[muxInd].mux,buf);
			//printf("%s ",list[muxInd].mux);
			list[muxInd].muxinp=(char**)calloc(uniqfuCount+1,sizeof(char*));
			for(k=0,l=0;k<fuCount;k++){
				if(uniqfu[k]){
					list[muxInd].muxinp[l]=(char*)calloc(10,sizeof(char));
					strcpy(list[muxInd].muxinp[l],fuList[k].fu);
					//printf("%s,", list[muxInd].muxinp[l]);
					l++;
				}

			}
			list[muxInd].muxinp[l]=(char*)calloc(5,sizeof(char));
			strcpy(list[muxInd].muxinp[l],"NULL");

			strcpy(list[muxInd].muxout,regList[i].reg);
			//printf(" %s ",list[muxInd].muxout );
			list[muxInd].contsig=(char**)calloc(uniqfuCount+1,sizeof(char*));
			for(l=0;l<uniqfuCount;l++){
					list[muxInd].contsig[l]=(char*)calloc((int)ceil(log(uniqfuCount)/log(2))+1,sizeof(char));
					strcpy(list[muxInd].contsig[l],decTobin(l,(int)ceil(log(uniqfuCount)/log(2))));
					//printf("%s,",list[muxInd].contsig[l]);
			}
			//printf("\n");
			muxInd++;
		}
	}
	muxCount=muxInd;
	strcpy(list[muxInd].mux,"NULL"); //added by zaki
	return list;
}
void swap(countsToMinimizemux *xp, countsToMinimizemux *yp)
{
    countsToMinimizemux temp = *xp;
    *xp = *yp;
    *yp = temp;
}
 
// A function to implement bubble sort
void bubbleSort(countsToMinimizemux arr[], int n)
{
   int i, j;
   for (i = 0; i < n-1; i++)      
 
       // Last i elements are already in place   
       for (j = 0; j < n-i-1; j++) 
           if (arr[j].countSum < arr[j+1].countSum){
              swap(&arr[j], &arr[j+1]);
           }
}

void minimizeMux(Register *regList,FU *fuList,RToperation *rtList){
	
	int indFu,indReg,indRTope,i,j,check,checkRight,checkLeft,checkEqual;
	int RightNotECount,LeftNotECount;

	for(indFu=0;indFu<fuCount;indFu++){

		//creating counts array & intializing it
		countsToMinimizemux counts[regCount];
		for(indReg=0;indReg<regCount;indReg++){
			strcpy(counts[indReg].reg,regList[indReg].reg);
			counts[indReg].leftCount=0;
			counts[indReg].rightCount=0;
			counts[indReg].countSum=0;
		}

		//counting leftcount,rightcount and sum of them
		for(indRTope=0;indRTope<RTopeCount;indRTope++){
			if(!strcmp(rtList[indRTope].opetype,"A") && !strcmp(fuList[indFu].fu,rtList[indRTope].oprep.assopr.fu)){
				for(indReg=0;indReg<regCount;indReg++){
					if(!strcmp(regList[indReg].reg,rtList[indRTope].oprep.assopr.leftoperand)){
						counts[indReg].leftCount++;
						counts[indReg].countSum++;
					}
					if(!strcmp(regList[indReg].reg,rtList[indRTope].oprep.assopr.rightoperand)){
						counts[indReg].rightCount++;
						counts[indReg].countSum++;
					}
				}
			}
		}
		//sorting based on sum of both leftcount and rightcount
		bubbleSort(counts,regCount);
		

		for(indReg=0;indReg<regCount;indReg++){

			//counting left and right registers			
			for(i=0;i<regCount;i++){
				counts[i].leftCount=0;
				counts[i].rightCount=0;
			}
			for(indRTope=0;indRTope<RTopeCount;indRTope++){
				if(!strcmp(rtList[indRTope].opetype,"A") && !strcmp(fuList[indFu].fu,rtList[indRTope].oprep.assopr.fu)){
						

					for(i=0;i<regCount;i++){
						if(!strcmp(counts[i].reg,rtList[indRTope].oprep.assopr.leftoperand))
							counts[i].leftCount++;
						if(!strcmp(counts[i].reg,rtList[indRTope].oprep.assopr.rightoperand))
							counts[i].rightCount++;
					}
				}	
			}
			printf("%s \n",fuList[indFu].fu );
			for(i=0;i<regCount;i++)
				printf("%s %d %d %d\n",counts[i].reg,counts[i].leftCount,counts[i].rightCount,counts[i].countSum);	
			printRTList(rtList);
			
			if(counts[indReg].leftCount >= 1 && counts[indReg].rightCount >= 1){
				
				RightNotECount=0;LeftNotECount=0;
				//which way to swap left or right
				for(j=0;j<RTopeCount;j++){
					//right  not eligible count
					if(!strcmp(rtList[j].opetype,"A") && !strcmp(fuList[indFu].fu,rtList[j].oprep.assopr.fu)){
						if(!strcmp(counts[indReg].reg,rtList[j].oprep.assopr.leftoperand)){
							//shouldn't swap already swapped registers
							check=0;
							for(i=indReg;i>0;){
								if(!strcmp(counts[--i].reg,rtList[j].oprep.assopr.rightoperand)){
									RightNotECount++;
									break;
								}
							}
						}

						//left not eligible count
						if(!strcmp(counts[indReg].reg,rtList[j].oprep.assopr.rightoperand)){
							//shouldn't swap already swapped registers
							check=0;
							for(i=indReg;i>0;){
								if(!strcmp(counts[--i].reg,rtList[j].oprep.assopr.leftoperand)){
									LeftNotECount++;
									break;
								}
							}
						}
					}
				}
				printf("********************: %d %d\n",LeftNotECount,RightNotECount );
				for(indRTope=0;indRTope<RTopeCount;indRTope++)
				{
					if(!strcmp(rtList[indRTope].opetype,"A") && !strcmp(fuList[indFu].fu,rtList[indRTope].oprep.assopr.fu)){
						//left not eligible count is less so move right to left
						if(LeftNotECount < RightNotECount){
							if(!strcmp(counts[indReg].reg,rtList[indRTope].oprep.assopr.rightoperand)){
								//shouldn't swap already swapped registers
								check=0;
								for(i=indReg;i>0;){
									if(!strcmp(counts[--i].reg,rtList[indRTope].oprep.assopr.leftoperand)){
										check=1;
										break;
									}
								}
								if(check)
									continue;
								//swap 
								char temp[5];
								strcpy(temp,rtList[indRTope].oprep.assopr.rightoperand);
								strcpy(rtList[indRTope].oprep.assopr.rightoperand,rtList[indRTope].oprep.assopr.leftoperand);
								strcpy(rtList[indRTope].oprep.assopr.leftoperand,temp);
							}
						}
						//right not eligible count is less so move left to right
						else{
							if(!strcmp(counts[indReg].reg,rtList[indRTope].oprep.assopr.leftoperand)){
								//shouldn't swap already swapped registers
								check=0;
								for(i=indReg;i>0;){
									if(!strcmp(counts[--i].reg,rtList[indRTope].oprep.assopr.rightoperand)){
										check=1;
										break;
									}
								}
								if(check)
									continue;
								//swap 
								char temp[5];
								strcpy(temp,rtList[indRTope].oprep.assopr.rightoperand);
								strcpy(rtList[indRTope].oprep.assopr.rightoperand,rtList[indRTope].oprep.assopr.leftoperand);
								strcpy(rtList[indRTope].oprep.assopr.leftoperand,temp);
							}
						}
					}	
				}
			}
		}
		/*
		printf("%s \n",fuList[indFu].fu );
		for(indReg=0;indReg<regCount;indReg++){
			printf("%s %d %d %d\n",counts[indReg].reg,counts[indReg].leftCount,counts[indReg].rightCount,counts[indReg].countSum);
		}
		*/
		
	}
}


char* removeSpace(char *str){
	char *end=str+strlen(str)-1;
	while(*str==' ' && ++str);
	while(*end==' ' && --end);
	*(end+1)='\0';
	return str;
}

//Function for parsing registers from file
Register *parseReg(FILE *inp,int initialReg){
	printf("\nRegisters\n\n");
	regSize=initialReg;
	char line[100]={},*token;
	int i=0;
	fscanf(inp,"%[^\n]",line);fgetc(inp);
	Register *list=(Register *)calloc(initialReg,sizeof(Register));
	while(!strchr(line,'<') && !strchr(line,'>')){
		token=strtok(line,",");token=removeSpace(token);
		strcpy(list[i].reg,token);
		token=strtok(NULL,",");
		list[i].width=atoi(token);
		printf("%s %d \n", list[i].reg,list[i].width);
		fscanf(inp,"%[^\n]",line);fgetc(inp);
		i++;
		if(i>=regSize){
			regSize*=2;
			list=realloc(list,sizeof(Register)*regSize);
		}
	}
	regCount=i;
	strcpy(list[i].reg,"NULL");	///added by zaki
	return list;
}

//Function for parsing functional units from file
FU *parseFu(FILE *inp,int initialFu){
	printf("\nFU's\n\n");
	fuSize=initialFu;
	char line[100]={},*token;
	int i=0,opCou;
	fscanf(inp,"%[^\n]",line);fgetc(inp);
	FU *list=(FU *)calloc(initialFu,sizeof(FU));
	while(!feof(inp)){
		opCou=0;
		token=strtok(line,",");token=removeSpace(token);
		strcpy(list[i].fu,token);
		token=strtok(NULL,",");token=removeSpace(token);token=removeSpace(token+1);
		strcpy(list[i].oper[opCou++],token);
		while(!strchr(token,')')){
			token=strtok(NULL,",");token=removeSpace(token);
			strcpy(list[i].oper[opCou++],token);
		}
		strcpy(list[i].oper[opCou],"NULL");// added by zaki
		list[i].oper[opCou-1][strlen(list[i].oper[opCou-1])-1]='\0';
		token=strtok(NULL,",");token=removeSpace(token);
		list[i].out_wid=atoi(++token);
		token=strtok(NULL,",");token=removeSpace(token);
		list[i].in1_wid=atoi(token);
		token=strtok(NULL,",");token=removeSpace(token);
		token[strlen(token)-1]='\0';
		list[i].in2_wid=atoi(token);
		printf(":%s: :%s: :%s: %d %d %d\n",list[i].fu,list[i].oper[0],list[i].oper[1],list[i].out_wid,list[i].in1_wid,list[i].in2_wid);
		line[0]='\0';
		fscanf(inp,"%[^\n]",line);fgetc(inp);
		//printf(":%s:\n", line);
		i++;
		if(i>=fuSize){
			fuSize*=2;
			list=realloc(list,sizeof(FU)*fuSize);
		}
		if(strlen(line)==0){
			printf("rt parsing is end\n");
			break;
		}
	}
	fuCount=i;
	strcpy(list[i].fu,"NULL");	///added by zaki
	return list;
}
void printRTList(RToperation *list){
	printf("\nRT Operations\n\n");
	int i;
	for(i=0;i<RTopeCount;i++){
		printf("%s %d %s ",list[i].block,list[i].timestamp,list[i].opetype);
		if(!strcmp(list[i].opetype,"R") || !strcmp(list[i].opetype,"O"))
			printf("%s %s %d\n",list[i].oprep.rwopr.port,list[i].oprep.rwopr.ioreg,list[i].oprep.rwopr.io_wid );
		else{
			printf(":%s: %d ",list[i].oprep.assopr.target, list[i].oprep.assopr.target_wid);
			printf(":%s: %d ",list[i].oprep.assopr.leftoperand, list[i].oprep.assopr.left_wid);
			printf(":%s: :%s: ", list[i].oprep.assopr.fu,list[i].oprep.assopr.fuoper);
			printf(":%s: %d \n", list[i].oprep.assopr.rightoperand,list[i].oprep.assopr.right_wid);
		}
	}
}

//parsing RT operations from file
RToperation *parseRToper(FILE *inp,int initialope){
	//printf("\nOperations\n\n");
	RTopeSize=initialope;
	char line[100]={},*token;
	int i=0;
	fscanf(inp,"%[^\n]",line);fgetc(inp);
	fscanf(inp,"%[^\n]",line);fgetc(inp);
	RToperation *list=(RToperation *)calloc(initialope,sizeof(RToperation));
	//while(!feof(inp))
	while(!strchr(line,'<') && !strchr(line,'>')){
		printf(":%s: %d\n", line,i);
		token=strtok(line,",");token=removeSpace(token);
		strcpy(list[i].block,token);
		token=strtok(NULL,",");token=removeSpace(token);
		list[i].timestamp=atoi(token);
		token=strtok(NULL,",");token=removeSpace(token);
		strcpy(list[i].opetype,token);
		//printf("operation:%d:\n",i );
		if(!strcmp(list[i].opetype,"R") || !strcmp(list[i].opetype,"O")){
			token=strtok(NULL,",");token=removeSpace(token);token=removeSpace(token+1);
			strcpy(list[i].oprep.rwopr.port,token);
			token=strtok(NULL,",");token=removeSpace(token);
			strcpy(list[i].oprep.rwopr.ioreg,token);
			token=strtok(NULL,",");token=removeSpace(token);
			token[strlen(token)-1]='\0';
			list[i].oprep.rwopr.io_wid=atoi(token);
			//printf(":%s: :%s: %d\n",list[i].oprep.rwopr.port,list[i].oprep.rwopr.ioreg,list[i].oprep.rwopr.io_wid);
		}
		else if(!strcmp(list[i].opetype,"A")){
			token=strtok(NULL,",");token=removeSpace(token);token=removeSpace(token+1);
			strcpy(list[i].oprep.assopr.target,token);
			token=strtok(NULL,",");token=removeSpace(token);
			token[strlen(token)-1]='\0';
			list[i].oprep.assopr.target_wid=atoi(token);

			//printf(":%s: %d ",list[i].oprep.assopr.target, list[i].oprep.assopr.target_wid);

			token=strtok(NULL,",");token=removeSpace(token);token=removeSpace(token+1);
			strcpy(list[i].oprep.assopr.leftoperand,token);
			token=strtok(NULL,",");token=removeSpace(token);
			token[strlen(token)-1]='\0';
			list[i].oprep.assopr.left_wid=atoi(token);

			//printf(":%s: %d ",list[i].oprep.assopr.leftoperand, list[i].oprep.assopr.left_wid);

			token=strtok(NULL,",");token=removeSpace(token);token=removeSpace(token+1);
			strcpy(list[i].oprep.assopr.fu,token);
			token=strtok(NULL,",");token=removeSpace(token);
			token[strlen(token)-1]='\0';
			strcpy(list[i].oprep.assopr.fuoper,token);

			//printf(":%s: :%s: ", list[i].oprep.assopr.fu,list[i].oprep.assopr.fuoper,token);

			token=strtok(NULL,",");token=removeSpace(token);token=removeSpace(token+1);
			strcpy(list[i].oprep.assopr.rightoperand,token);
			token=strtok(NULL,",");token=removeSpace(token);
			token[strlen(token)-1]='\0';
			list[i].oprep.assopr.right_wid=atoi(token);

			//printf(":%s: %d \n", list[i].oprep.assopr.rightoperand,list[i].oprep.assopr.right_wid);
		}
		line[0]='\0';
		fscanf(inp,"%[^\n]",line);fgetc(inp);
		i++;
		if(i>=RTopeSize){
			RTopeSize*=2;
			list=realloc(list,sizeof(RToperation)*RTopeSize);
		}
		if(strlen(line)==0){
			printf("rt parsing is end\n");
			break;
		}
	}
	RTopeCount=i;
	strcpy(list[i].block,"NULL");  //added by zaki
	return list;
}
