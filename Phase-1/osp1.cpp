#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using namespace std;

fstream inF = fstream("input.txt",ios::in);
fstream outF = fstream("outputFin.txt",ios::out);

//Virtual Machine 
char M[100][4];
char IR[4];
int IC;
char R[4];
bool C;
int SI;

void masterMode(int SI);
void load();
void clearMemory();
void startJobExecution();
void readProgramBlock(int &m,string line);
void readData(int &m,string line);
void writeData(int m);
void terminateJob();
int calculate_m_fromIR();

int calculate_m_fromIR(){ // Calculate the XX of instructions, such as GDXX
    int m = (IR[2]-'0') * 10 + ( IR[3]-'0' ) ;
    return m;
}

void clearMemory(){
    int m=0;
    for(int i=0;i<100;i++)
    {
        for(int j=0;j<4;j++)
        {
            M[i][j] = ' ';
        }
    }
    memset(IR,' ',4);
    IC = 0;
    memset(R,' ',4);
    SI=0;
    C = false;
}

void readProgramBlock(int &m,string line){
    int i = 0;
    for(char c : line)
    {
        if(c=='H')
        {
            if(i != 0)
            {
                while(i < 4)
                {
                    M[m][i] = ' ';
                    i++;
                }
                m++;
                i = 0;
            }
            M[m][0]=c;
            M[m][1]=' ';
            M[m][2]=' ';
            M[m][3]=' ';
            m++;
        }
        else
        {
            M[m][i] = c;
            i++;
            if(i==4){
                i=0;
                m++;
            }
        }
    }
}

void readData(int &m,string line){
    int i =0;
    int target = m+9; // x0-x9 , blocks of 10 (eg:10-19)
    for(char ch : line)
    {
        M[m][i] = ch;
        i++;
        if(i==4){
            m++;
            i = 0;
        }
        if(m > target) return;
    }
    //fill the rest of memory with ' '
    while(m<=target){
        M[m][i] = ' ';
        i++;
        if(i==4){
            m++;
            i = 0;
        }
    }
}

void writeData(int m)
{
    int count = 0;
    int i = 0;
    while(count<40)
    {
        outF<<M[m][i];
        i++;
        count++;
        if(i==4){
            m++;
            i=0;
        }
    }
    outF<<endl;
}

void terminateJob(){
    //2 blank lines after completion of job;
    outF<<endl<<endl;
    return;
}

void masterMode(int SI){
    string line = "" ;
    int m = 0;
    switch(SI)
    {
        case 1: // read;
            m = calculate_m_fromIR(); 
            getline(inF,line);
            readData(m,line);
            break;
        case 2: // write;
            m = calculate_m_fromIR();
            writeData(m);
            break;
        case 3: // terminate; H
            terminateJob();
            break;
        default: cout<<"Wrong initialization of SI";
            break;
    }
}

void startJobExecution(){
    IC = 00;
    int jmp = false;
    int flag = 1 ; //check for halt, if halt: flag becomes 0
    do{ 
        memcpy(IR,M[IC],4);
        if( IR[0] == 'G' && IR[1] == 'D'){ //GDXX
            SI = 1;
            IC++;
            masterMode(SI);
            continue;
        }
        else if( IR[0]=='P' && IR[1] == 'D'){ //PDXX
            SI = 2;
            IC++;
            masterMode(SI);
            continue;
        }
        else if( IR[0]=='H' ){ //H
            SI = 3;
            flag = 0;
            masterMode(SI);
        }
        else if( IR[0]=='L' && IR[1] == 'R' ){ // LRXX
            int m = calculate_m_fromIR();
            memcpy(R,M[m],4);
        }
        else if( IR[0]=='S' && IR[1] == 'R' ){ // SRXX
            int m = calculate_m_fromIR();
            memcpy(M[m],R,4);
        }
        else if( IR[0]=='C' && IR[1] =='R' ){ //CRXX
            int m = calculate_m_fromIR();
            C = ( memcmp(R,M[m],4) == 0 );
        }
        else if( IR[0]=='B' && IR[1] =='T' ){ //BTXX
            if(C){
                int m = calculate_m_fromIR();
                IC = m;
                jmp = true;
            }
        }
        if(!jmp){
            IC++;
        }
        jmp = false;
    }while(flag);
}

void load()
{
    int m =0,i=0 ;
    string line;
    string jobStr = "$AMJ";
    string dataStr = "$DTA";
    string endStr = "$END";
    char* buff[40];
    while(getline(inF,line))
    {
        //check if card is job card : $AMJ
        if(line.substr(0,4)==jobStr)
        {
            m = 0;
            clearMemory();
            continue;
        }
        //check if execution card  : $DTA
        else if(line.substr(0,4)==dataStr)
        {
            startJobExecution();
            m = 0 ;
            continue;
        }
        //check if end card : $END
        else if(line.substr(0,4)==endStr)
        {
            continue;
        }
        //read the instructions, eg : gd10pb10h
        else{
            readProgramBlock(m,line);
        }
        
    }
}

int main()
{
    cout<<"Reading input from file\n";
    
    if(!inF.is_open())
    {
        cout<<"Error reading input file : input file not found \n";
        return 0;
    }
    if(!outF.is_open())
    {
        cout<<"Error writing output"<<"\n";
        return 0;
    }
    cout<<"Processing input \n";
    load();
    cout<<"Ouput written to output.txt\n";
    return 0;
}