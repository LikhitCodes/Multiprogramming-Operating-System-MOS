    #include <iostream>
    #include <fstream>
    #include <cstring>
    #include <string>
    #include <random>

    using namespace std;

    //fstream inF = fstream("input.txt",ios::in);
    fstream inF;
    fstream outF = fstream("outputFin.txt",ios::out);

    //Virtual Machine 
    char M[300][4];
    char IR[4];
    int IC;
    char R[4];
    bool C;

    //Interrupts
    int SI;
    int PI;
    int TI;

    //pagin
    int PTR;
    int VA;
    int RA;
    int TTL;
    int TLL;

    //Error codes
    enum EM{
        No_Error,
        OUT_OF_DATA,
        LINE_LIMIT_EXCEEDED,
        TIME_LIMIT_EXCEEDED,
        OPERATION_CODE_ERROR,
        OPERAND_ERROR,
        INVALID_PAGE_FAULT
    };

    struct PCB{
        int pid;
        int TTL;
        int TLL;
        int TTC;
        int LLC;
    };

    PCB pcb;

    bool frameUsed[30];

    void masterMode();
    void load();
    void clearMemory();
    void startExecution();
    void ExecuteUserProgram();
    void readProgramBlock(string line);
    void readData(int &m,string line);
    void writeData(int m);
    void terminateJob(EM error, EM error2 = No_Error);
    int calculate_m_fromIR();
    int allocateFrame();
    void updatePageTable(int pageNumber,int frame);
    int addressMap();
    void hasTimeExceeded();
    void hasLineExceeded();

    bool terminated;

    int calculate_m_fromIR(){ // Calculate the XX of instructions, such as GDXX
        int m = (IR[2]-'0') * 10 + ( IR[3]-'0' ) ;
        return m;
    }

    void clearMemory(){
        int m=0;
        for(int i=0;i<300;i++)
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
        TI =0;
        PI=0;
        C = false;
        pcb.LLC = 0;
        pcb.TLL = 0;
        pcb.TTC = 0;
        pcb.TTL = 0;
        pcb.pid = -1;
        PTR = 0;
        VA =0;
        RA =0;
        terminated = false;
        for(int i=0; i<30; i++){
            frameUsed[i] = false;
        }
    }

    int allocateFrame(){
        int frame;
        do{
            frame = rand()%30;      //stop the loop when the allocated frame is not in use
        }while(frameUsed[frame]);  
        frameUsed[frame] = true;
        return frame;
    }

    void updatePageTable(int pageNumber,int frame){
        int addr = PTR + pageNumber;
        M[addr][0] = '1';
        M[addr][1] = (frame/10) + '0';
        M[addr][2] = (frame%10) + '0';
        M[addr][3] = ' ';
    }

    void readProgramBlock(string line){
        int frame,m ;
        int wordCnt = 0;
        int pageCnt = 0;
        int i = 0;
        for(char c : line)
        {
            if(i==0 && wordCnt%10 ==0){
                frame = allocateFrame();
                updatePageTable(pageCnt,frame);
                pageCnt++;
                m = frame*10;
            }
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
                wordCnt++;
                i=0;
            }
            else
            {
                M[m][i] = c;
                i++;
                if(i==4){
                    i=0;
                    m++;
                    wordCnt++;
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

    void terminateJob(EM error, EM error2){
        terminated = true;
        //handling 2 error cases
        if(error2 != No_Error){
            if(error == TIME_LIMIT_EXCEEDED && error2 == OPERATION_CODE_ERROR){
                outF<<"Time limit exceed error(3)";
                outF<<"& operation code error(4) occured in job id "<<pcb.pid;
                outF<<endl<<endl;
            }
            else if(error == TIME_LIMIT_EXCEEDED && error2 == OPERAND_ERROR){
                outF<<endl<<endl;
                outF<<"Time limit exceed error(3)";
                outF<<"& operand error occured(5) in job id "<<pcb.pid;
                outF<<endl<<endl;
            }
        }else{
            outF<<"Line Count:"<<pcb.LLC<<" Time Taken: "<<pcb.TTC<<endl;
            switch (error)
            {
            case No_Error:
                outF<<"Completed with no errors, job id "<<pcb.pid;
                outF<<endl<<endl;
                break;

            case INVALID_PAGE_FAULT:
                outF<<"Invalid page fault error occured in job id "<<pcb.pid;
                outF<<endl<<endl;
                break;

            case OPERAND_ERROR:
                outF<<"operand error occured in job id "<<pcb.pid;
                outF<<endl<<endl;
                break;

            case OPERATION_CODE_ERROR:
                outF<<"operation error occured in job id "<<pcb.pid;
                outF<<endl<<endl;
                break;

            case LINE_LIMIT_EXCEEDED:
                outF<<"line limit exceeded error occured in job id "<<pcb.pid;
                outF<<endl<<endl;
                break;

            case TIME_LIMIT_EXCEEDED:
                outF<<"time limit exceeded error occured in job id "<<pcb.pid;
                outF<<endl<<endl;
                break;
            
            case OUT_OF_DATA:
                outF<<"out of data error occured in job id "<<pcb.pid;
                outF<<endl<<endl;
                break;
            
            default:
                break;
            }
            //outF<<endl;
        }
        
    }

    void masterMode(){
        string line = "" ;
        int m = 0;

        //handling INVALID_PAGE_FAULT cases
        if(PI != 0){
            //TI = 0 cases
            if(TI == 0){
                if(PI == 1){
                    terminateJob(OPERATION_CODE_ERROR);
                }
                else if(PI == 2){
                    terminateJob(OPERAND_ERROR);
                }
                else if(PI==3){
                    if((IR[0]=='G' && IR[1]=='D') || (IR[0]=='S' && IR[1]=='R')){ //safe page fault
                        int pageNumber = VA / 10;
                            int frame = allocateFrame();
                            updatePageTable(pageNumber, frame);
                            PI = 0;
                            IC--; 
                    }
                    else{
                        terminateJob(INVALID_PAGE_FAULT);
                    }
                }
            }
            else if(TI == 2){
                if(PI == 1){
                    terminateJob(TIME_LIMIT_EXCEEDED,OPERATION_CODE_ERROR);
                }
                else if(PI == 2){
                    terminateJob(TIME_LIMIT_EXCEEDED,OPERAND_ERROR);
                }
                else if(PI==3){
                    terminateJob(TIME_LIMIT_EXCEEDED);
                }
            }
            return;
        }
        //HAndling SI when TI = 0
        if(TI == 0){
            if(SI==1)  // read;
            {
                    VA = calculate_m_fromIR();
                    RA = addressMap();
                    while(PI == 3){  // handle valid page fault
                        int pageNumber = VA / 10;
                        int frame = allocateFrame();
                        updatePageTable(pageNumber, frame);
                        PI = 0;
                        RA = addressMap();  // retry address map
                    }
                    if(PI != 0){
                        masterMode();  // handle invalid page fault
                        return;
                    }
                    getline(inF,line);
                    streampos pos = inF.tellg(); //save position before reading
                    if(line.substr(0,4)=="$END" ||
                    line.substr(0,4)=="$AMJ" ||
                    line.substr(0,4)=="$DTA"){
                        inF.seekg(pos);  // return to the program card
                        terminateJob(OUT_OF_DATA);
                        return;
                    }
                    readData(RA,line);
            }
            else if(SI==2){  //write;
                    VA = calculate_m_fromIR();
                    RA = addressMap();
                    if(PI != 0) {
                        masterMode(); 
                        return; 
                    }
                    pcb.LLC++;
                    if(pcb.LLC > pcb.TLL){
                        terminateJob(LINE_LIMIT_EXCEEDED);
                        return;
                    }
                    writeData(RA);
            }
            else if(SI==3){  //Halt ;
                    terminateJob(No_Error);
            }
            return;
        }
        if(TI==2){
            TI = 0;
            if(SI == 1){   // Read
                terminateJob(TIME_LIMIT_EXCEEDED);
            }

            else if(SI == 2){   // WRte
                VA = calculate_m_fromIR();
                RA = addressMap();
                pcb.LLC++;                                
                if(pcb.LLC > pcb.TLL){                  
                    terminateJob(LINE_LIMIT_EXCEEDED);  
                    return;
                }
                writeData(RA);
                terminateJob(TIME_LIMIT_EXCEEDED);
            }

            else if(SI == 3){   // Halt
                terminateJob(No_Error);
            }
        }
        SI = 0;
    }

    void startExecution(){
        IC = 00;
        ExecuteUserProgram();
    }

    int addressMap(){
        PI = 0;
        if(VA < 0 || VA >=100){
            PI = 2;
            return -1;
        }
        int page = VA / 10;
        int offset = VA % 10;
        int pte = PTR + page;
        if(M[pte][0] != '1'){
            PI = 3;
            return -1;
        }
        int frame = (M[pte][1]-'0') * 10 + (M[pte][2] - '0');
        RA = frame*10 + offset ;
        return RA;
    }

    void hasTimeExceeded(){
        if (pcb.TTC>pcb.TTL)
            TI = 2;
    }

    void hasLineExceeded(){
        if (pcb.LLC>pcb.TLL)
            terminateJob(LINE_LIMIT_EXCEEDED);
    }

    void ExecuteUserProgram(){
        int jmp = false;
        int flag = 1 ; //check for halt, if halt: flag becomes 0
        while(flag && !terminated){ 
            VA = IC;
            RA = addressMap();
            if(PI != 0){
                if(PI == 3){
                    int pageNumber = VA / 10;
                    int frame = allocateFrame();
                    updatePageTable(pageNumber, frame);
                    PI = 0;
                    IC--;
                } else {
                    masterMode();
                }
                continue;
            }
            if(TI != 0){                          
                terminateJob(TIME_LIMIT_EXCEEDED); 
                continue;                          
            }
            memcpy(IR,M[RA],4);
            IC++;
            if( IR[0] == 'G' && IR[1] == 'D'){ //GDXX
                SI = 1;
                pcb.TTC++;
                hasTimeExceeded();
                masterMode();
                continue;
            }
            else if( IR[0]=='P' && IR[1] == 'D'){ //PDXX
                SI = 2;
                pcb.TTC++;
                hasTimeExceeded();
                masterMode();
                continue;
            }
            else if( IR[0]=='H' ){ //H
                SI = 3;
                flag = 0;
                continue;
            }
            else if( IR[0]=='L' && IR[1] == 'R' ){ // LRXX
                VA = calculate_m_fromIR();
                RA = addressMap();
                SI = 0;
                if(PI==3){
                    PI = 2;
                    masterMode();
                    continue;
                }
                if(PI!=0 || TI!=0){
                    masterMode();
                    continue;
                }
                pcb.TTC++;
                hasTimeExceeded();
                memcpy(R,M[RA],4);
            }
            else if( IR[0]=='S' && IR[1] == 'R' ){ // SRXX
                VA = calculate_m_fromIR();
                RA = addressMap();
                if(PI!=0|| TI!=0){
                    masterMode();
                    continue;
                }
                pcb.TTC++;
                hasTimeExceeded();
                memcpy(M[RA],R,4);
                continue;
            }
            else if( IR[0]=='C' && IR[1] =='R' ){ //CRXX
                VA = calculate_m_fromIR();
                RA = addressMap();
                if(PI==3){
                    PI = 2;
                    masterMode();
                    continue;
                }
                if(PI!=0|| TI!=0){
                    masterMode();
                    continue;
                }
                pcb.TTC++;
                hasTimeExceeded();
                C = ( memcmp(R,M[RA],4) == 0 );
            }
            else if( IR[0]=='B' && IR[1] =='T' ){ //BTXX
                if(TI != 0){
                    masterMode();
                    continue;
                }
                pcb.TTC++;
                hasTimeExceeded();
                if(C){
                    IC = calculate_m_fromIR();
                    jmp = true;
                }
            }
            else{
                PI = 1;
                SI = 0;
                masterMode();
                flag=0;
            }
            jmp = false;
        }
        if(flag==0 && !terminated){
            masterMode();
        }
    }

    void initialize_pcb(PCB &process,string line){
        process.pid = stoi(line.substr(4,4));
        process.TTL = stoi(line.substr(8,4));
        process.TLL = stoi(line.substr(12,4));
        process.TTC = 0;
        process.LLC = 0;
    }

    void initialize_pageTable(){
        for(int i = PTR; i < PTR + 10; i++){
        for(int j = 0; j < 4; j++){
            M[i][j] = '*';
        }
        }
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
                clearMemory();
                m = 0;
                memset(frameUsed,false,sizeof(frameUsed));
                string jobId = line.substr(4,4);
                if(line.length() != 16){
                    cout<<"The job with id : "<<jobId<<" has invalid program card so skipping execution ";
                    while (getline(inF,line))
                    {
                        if(line.substr(0,4)!=endStr){
                            continue;
                        }
                    }
                }
                else{
                    initialize_pcb(pcb,line);
                    PTR = ( rand()%30 ) * 10;
                    frameUsed[PTR/10] = true;
                    initialize_pageTable();
                }
                continue;
            }
            //check if execution card  : $DTA
            else if(line.substr(0,4)==dataStr)
            {
                startExecution();
                continue;
            }
            //check if end card : $END
            else if(line.substr(0,4)==endStr)
            {
                continue;
            }
            //read the instructions, eg : gd10pb10h
            else{
                readProgramBlock(line);
            }
        }
    }

    int main()
    {
        string fname;
        cout<<"Enter input file name: (for default press enter)";
        getline(cin,fname);
        if(!fname.empty()){
            inF = fstream(fname,ios::in);
        }else{
            inF = fstream("input.txt",ios::in);
        }
        srand(time(0));
        cout<<"\nReading input from file\n";
        
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