#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
extern "C"{
    void myPrint_asm(const char* str,int len);
    void myPrintRed_asm(const char* str,int len);
}
void myPrint(const string& s){
    myPrint_asm(s.c_str(),(int)s.length());
    //cout<<s;
}
void myPrint_red(const string& s){
    myPrintRed_asm(s.c_str(),(int)s.length());
    //cout<<s;
}
void lineSeperator(){
    char c = 0xa;
    myPrint_asm(&c, 1);
    //cout<<endl;
}
class clust{
public:
    char buffer[512]{};
    explicit clust(const char* b){
        for(int i=0;i<512;i++){
            buffer[i] = b[i];
        }
    }
};
class BOOT_HEAD{
public:
    char BS_jmpBOOT[3]{};
    char BS_OEMName[8]{};
    char BPB_BytesPerSec[2]{};
    char BPB_SecPerClus{};
    char BPB_ResvdSecCnt[2]{};
    char BPB_NumFATs{};
    char BPB_RootEntCnt[2]{};
    char BPB_TotSec16[2]{};
    char BPB_Media{};
    char BPB_FATSz16[2]{};
    char BPB_SecPerTrk[2]{};
    char BPB_NumHeads[2]{};
    char BPB_HiddSec[4]{};
    char BPB_TotSec32[4]{};
    char BS_DrvNum{};
    char BS_Reserved1{};
    char BS_BootSig{};
    char BS_VollD[4]{};
    char BS_VolLab[11]{};
    char BS_FileSysType[8]{};
    char AsmCode[448]{};
    char EndMark[2]{};
    BOOT_HEAD() = default;
    BOOT_HEAD(clust c){
        int idx = 0;
        for(char & i : BS_jmpBOOT){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BS_OEMName){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BPB_BytesPerSec){
            i = c.buffer[idx];
            idx++;
        }
        BPB_SecPerClus = c.buffer[idx];
        idx++;
        for(char & i : BPB_ResvdSecCnt){
            i = c.buffer[idx];
            idx++;
        }
        BPB_NumFATs = c.buffer[idx];
        idx++;
        for(char & i : BPB_RootEntCnt){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BPB_TotSec16){
            i = c.buffer[idx];
            idx++;
        }
        BPB_Media = c.buffer[idx];
        idx++;
        for(char & i : BPB_FATSz16){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BPB_SecPerTrk){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BPB_NumHeads){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BPB_HiddSec){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BPB_TotSec32){
            i = c.buffer[idx];
            idx++;
        }
        BS_DrvNum = c.buffer[idx];
        idx++;
        BS_Reserved1 = c.buffer[idx];
        idx++;
        BS_BootSig = c.buffer[idx];
        idx++;
        for(char & i : BS_VollD){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BS_VolLab){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : BS_FileSysType){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : AsmCode){
            i = c.buffer[idx];
            idx++;
        }
        for(char & i : EndMark){
            i = c.buffer[idx];
            idx++;
        }
    }
};
class FAT_TABLE{
private:
    vector<unsigned int> fat_clust;
    char content[512*9]{};
    int part = 0;
public:
    void addClust(clust c);
    void init();
    int getNext(int idx){
        if(fat_clust[idx]>fat_clust.size()||fat_clust[idx]<2){
            return -1;
        }
        return fat_clust[idx];
    }
};

void FAT_TABLE::addClust(clust c) {
    for(int i=0;i<512;i++){
        content[i+part*512] = c.buffer[i];
    }
    part++;
}

void FAT_TABLE::init() {
    if(part<9){
        cout<<"need loading all 9 part!"<<endl;
    }
    else{
        int idx = 0;
        while(idx<512*9){
            unsigned char temp[3];
            for(unsigned char & i : temp){
                i = (unsigned char)content[idx];
                idx++;
            }

            unsigned int a = (((unsigned int)(temp[1])%16)<<8) + (unsigned int)(temp[0]);
            unsigned int b = (((unsigned int)(temp[1])>>4)) + (unsigned int)(temp[2]<<4);
            fat_clust.push_back(a);
            fat_clust.push_back(b);
        }
    }
}
class DIR_MSG{
public:
    char DIR_NAME[11]{};
    char DIR_Attr;
    char RESERVE[10]{};
    char DIR_WrtTime[2]{};
    char DIR_WrtDate[2]{};
    char DIR_FstClus[2]{};
    char DIR_FileSize[4]{};
    DIR_MSG():DIR_Attr(' '){}
    DIR_MSG(const char *dir,int begin){
        int idx = begin;
        for(char& i:DIR_NAME){
            i = dir[idx];
            idx++;
        }
        DIR_Attr = dir[idx];
        idx++;
        for(char& i:RESERVE){
            i = dir[idx];
            idx++;
        }
        for(char& i:DIR_WrtTime){
            i = dir[idx];
            idx++;
        }
        for(char& i:DIR_WrtDate){
            i = dir[idx];
            idx++;
        }
        for(char& i:DIR_FstClus){
            i = dir[idx];
            idx++;
        }
        for(char& i:DIR_FileSize){
            i = dir[idx];
            idx++;
        }
    }
};
class ROOT_DIR{
private:
    int root_size = 0;
    char* content{};
    int part = 0;
public:
    vector<DIR_MSG*> dirs;
    ROOT_DIR() = default;
    void init(int size);
    void addClust(clust& c);
    void read();
};
void ROOT_DIR::init(int size) {
    root_size = size;
    content = new char[512*((root_size*32+511)/512)];
}
void ROOT_DIR::addClust(clust &c) {
    for(int i=0;i<512;i++){
        content[i+part*512] = c.buffer[i];
    }
    part++;
}
void ROOT_DIR::read() {
    for(int i=0;i<root_size;i++){
        auto* temp = new DIR_MSG(content,i*32);
        if((int)temp->DIR_NAME[0]!=0&&(int)temp->DIR_Attr!=15)
            dirs.push_back(temp);
    }
}
class FAT_FILE{
private:
    string name;
    string content;
public:
    FAT_FILE() = default;
    FAT_FILE(DIR_MSG dirMsg,FAT_TABLE fat,vector<clust*>& imgFile,int start):name(dirMsg.DIR_NAME,11){
        int fat_num = (int)((unsigned char)dirMsg.DIR_FstClus[0]+(unsigned char)dirMsg.DIR_FstClus[1]*256);
        while(fat_num!=-1){
            int idx = start+(fat_num-2);
            content.append(imgFile[idx]->buffer,512);
            fat_num = fat.getNext(fat_num);
        }
    }
    string getName(){
        string res;
        int i = 7;
        int j = 10;
        for(;i>=0;i--){
            if(name[i]!=' '){
                break;
            }
        }
        for(;j>=8;j--){
            if(name[j]!=' '){
                break;
            }
        }
        res.append(name.substr(0,i+1)).append(".").append(name.substr(8,j-7));
        return res;
    }
    string getContent(){
        string res;
        int i = content.length()-1;
        for(;i>=0;i--){
            if(content[i]!=0){
                break;
            }
        }
        res.append(content.substr(0,i+1));
        return res;
    }
};
class FAT_DIR{
private:
    string name;
    vector<FAT_FILE*> files;
    vector<FAT_DIR*> subDirs;
public:
    string getName(){
        if(name.empty()){
            return "";
        }
        string res;
        int i = 7;
        int j = 10;
        for(;i>=0;i--){
            if(name[i]!=' '){
                break;
            }
        }
        for(;j>=8;j--){
            if(name[j]!=' '){
                break;
            }
        }
        res.append(name.substr(0,i+1)).append(name.substr(8,j-7));
        return res;
    }
    vector<FAT_DIR*>& getSubDirs(){
        return subDirs;
    }
    vector<FAT_FILE*>& getFiles(){
        return files;
    }
    void showWithNum(){
        if(!name.empty()){
            myPrint_red(".");
            lineSeperator();
            myPrint_red("..");
            lineSeperator();
        }
        for(FAT_DIR* d:subDirs){
            myPrint_red(d->getName());
            myPrint(" ");
            myPrint(to_string(d->subDirs.size()));
            myPrint(" ");
            myPrint(to_string(d->files.size()));
            lineSeperator();
        }
        for(FAT_FILE* f:files){
            myPrint(f->getName());
            myPrint(" ");
            myPrint(to_string(f->getContent().size()));
            lineSeperator();
        }
    }
    void show(){
        if(!name.empty()){
            myPrint_red(".  ..  ");
        }
        for(FAT_DIR* d:subDirs){
            myPrint_red(d->getName());
            myPrint("  ");
        }
        for(FAT_FILE* f:files){
            myPrint(f->getName());
            myPrint("  ");
        }
        lineSeperator();
    }
    FAT_DIR() = default;
    FAT_FILE* searchFileByName(const string& n){
        for(FAT_FILE* f:files){
            if(f->getName()==n){
                return f;
            }
        }
        return nullptr;
    }
    FAT_DIR* searchDirByName(const string& n){
        for(FAT_DIR* s:subDirs){
            if(s->getName()==n){
                return s;
            }
        }
        return nullptr;
    }
    FAT_DIR(DIR_MSG dirMsg,FAT_TABLE fat,vector<clust*>& imgFile,int start):name(dirMsg.DIR_NAME,11){
        int fat_num = (int)((unsigned char)dirMsg.DIR_FstClus[0]+(unsigned char)dirMsg.DIR_FstClus[1]*256);
        while(fat_num!=-1){
            int idx = start+(fat_num-2);
            for(int i=0;i<512;i+=32){
                DIR_MSG msg(imgFile[idx]->buffer, i);
                if(msg.DIR_Attr==16&&msg.DIR_NAME[0]!='.'){
                    auto temp = new FAT_DIR(msg,fat,imgFile,start);
                    subDirs.push_back(temp);
                }
                else if(msg.DIR_Attr==32||(msg.DIR_Attr==0&&msg.DIR_NAME[0]!=0)){
                    auto temp = new FAT_FILE(msg,fat,imgFile,start);
                    files.push_back(temp);
                }
            }
            fat_num = fat.getNext(fat_num);
        }
    }
    FAT_DIR(vector<FAT_FILE*> f,vector<FAT_DIR*> sub,string name){
        this->name = std::move(name);
        this->files = std::move(f);
        this->subDirs = std::move(sub);
    }
};
class FAT12{
private:
    BOOT_HEAD head;
    FAT_TABLE fat1,fat2;
    ROOT_DIR rootDir;
    vector<clust*> imgFile;
    string address;
    vector<FAT_FILE*> files;
    vector<FAT_DIR*> dirs;

public:
    FAT_DIR root;
    int data_start = 0;
    void setAddress(string ad);
    void load();
    void init();
    void readFile();
    void readDir();
};
void FAT12::setAddress(string ad) {
    address = std::move(ad);
}
void FAT12::load() {
    ifstream img(address,ios::binary);
    if(!img.is_open()){
        cout<<"fail to open";
        //读取失败
    }
    char buffer[512];
    while (!img.eof()){
        img.read(buffer,512);
        auto* temp = new clust(buffer);
        imgFile.push_back(temp);
    }
}
void FAT12::init() {
    BOOT_HEAD h(*imgFile[0]);
    head = h;
    for(int i=1;i<=9;i++){
        fat1.addClust(*imgFile[i]);
    }
    for(int i=10;i<=18;i++){
        fat2.addClust(*imgFile[i]);
    }
    fat1.init();
    fat2.init();
    int size = (int)((unsigned char)(head.BPB_RootEntCnt[0])+(unsigned char)(head.BPB_RootEntCnt[1])*256);
    rootDir.init(size);
    for(int i=19;i<int(19+(size*32+511)/512);i++){
        rootDir.addClust(*imgFile[i]);
    }
    rootDir.read();
    data_start = int(19+(size*32+511)/512);
    readFile();
    readDir();
    root = *new FAT_DIR(files,dirs,"");
}


void FAT12::readFile() {
    for(DIR_MSG* d:rootDir.dirs){
        if(d->DIR_Attr==32||(d->DIR_Attr==0&&d->DIR_NAME[0]!=0)){
            auto* file = new FAT_FILE(*d,this->fat1,this->imgFile,this->data_start);
            files.push_back(file);
        }
    }
}

void FAT12::readDir() {
    for(DIR_MSG* d:rootDir.dirs){
        if(d->DIR_Attr==16){
            auto* dir = new FAT_DIR(*d,this->fat1,this->imgFile,this->data_start);
            dirs.push_back(dir);
        }
    }
}
void start(){
    myPrint(">:");
}
vector<string> getPath(string s){
    vector<string> res;
    if(s.length()==0){
        return res;
    }
    char seperator = '/';
    int last_seperator = 0;
    if(s[0]==seperator){
        last_seperator = 1;
    }
    int len = 0;
    for(int i=0;i<s.length();i++){
        if(s.at(i)==seperator){
            if(len==0){
                continue;
            }
            res.push_back(s.substr(last_seperator,len));
            last_seperator = i+1;
            len = 0;
        }
        else{
            len++;
        }
    }
    res.push_back(s.substr(last_seperator));
    return res;
}
int isValid(const string& args){
    //0:invalid
    //1:ls
    //2:ls -l
    //3:地址重复
    if(args.empty()){
        return 1;
    }
    int flag = 0;
    int sum = 0;
    bool isL = false;
    for(char arg : args){
        if(arg=='-'){
            if(flag==0){
                flag = 1;
            }
            else{
                return 0;
            }
        }
        else if(arg==' '){
            flag = 0;
        }
        else{
            if(flag==0){
                continue;
            }
            else if(arg == 'l'){
                isL = true;
            }
            else{
                return 0;
            }
        }
    }
    for(int i=0;i<args.length()-1;i++){
        if((args[i]==' '&&args[i+1]!='-'&&args[i+1]!=' ')||(i==0&&args[i]!='-'&&args[i+1]!=' '&&args[i+1]!='-')){
            sum++;
            if(sum>1){
                return 3;
            }
        }
    }
    return isL?2:1;
}
bool isPath(const string& args,int idx){
    for(int i=idx;i<args.size();i++){
        if(args[i]=='-'){
            break;
        }
        else if(args[i]!=' '){
            return false;
        }
    }
    return true;
}
string getPathString(const string& args){
    if(args.empty()){
        return "";
    }
    if(args.length()==1){
        return args;
    }
    string res;
    int idx = 0;
    int len = 0;
    bool flag = false;
    for(int i=0;i<args.length()-1;i++){
        if(args[i]=='-'){
            if(isPath(args,i+1)){
                return "-";
            }
        }
        if(args[i]==' '&&args[i+1]!='-'&&args[i+1]!=' '){
            idx = i+1;
            flag = true;
        }
        if(i==0&&args[i]!='-'&&args[i+1]!=' '){
            idx = i;
            flag = true;
        }
    }
    if(!flag){
        return "";
    }
    for(int i = idx;i<args.length();i++){
        if(args[i]!=' '){
            len++;
        }
        else{
            break;
        }
    }
    res = args.substr(idx,len);
    return res;
}
void showWithNum(string name,FAT_DIR* dir){
    if(name[name.size()-1]!='/'){
        name.append("/");
    }
    name.append(dir->getName());
    myPrint(name);
    myPrint(" ");
    myPrint(to_string(dir->getSubDirs().size()));
    myPrint(" ");
    myPrint(to_string(dir->getFiles().size()));
    myPrint(":");
    lineSeperator();
    dir->showWithNum();
    for(FAT_DIR* d:dir->getSubDirs()){
        showWithNum(name,d);
    }
}
void showWithoutNum(string name,FAT_DIR* dir){
    if(name[name.size()-1]!='/'){
        name.append("/");
    }
    name.append(dir->getName());
    myPrint(name);
    myPrint(":");
    lineSeperator();
    dir->show();
    for(FAT_DIR* d:dir->getSubDirs()){
        showWithoutNum(name,d);
    }
}
void showLS(string args,FAT12& myFAT12){
    int valid = isValid(args);
    if(valid==0){
        myPrint("you have typed wrong parameter");
        lineSeperator();
        return;
    }
    if(valid==3){
        myPrint("you have typed path more than once");
        lineSeperator();
        return;
    }
    bool isLS_L = valid==2;
    string pathStr = getPathString(args);
    vector<string> path = getPath(std::move(pathStr));
    FAT_DIR* temp = &myFAT12.root;
    for(int i=0;i<path.size();i++){
        temp = temp->searchDirByName(path[i]);
        if(temp == nullptr){
            string wrong;
            wrong.append("there is no such dir:").append(path[i]).append("    please check your path");
            myPrint(wrong);
            lineSeperator();
            return;
        }
    }
    if(isLS_L){
        showWithNum("/",temp);
    }
    else{
        showWithoutNum("/",temp);
    }
}
void showCAT(string args,FAT12& myFAT12){
    vector<string> path = getPath(args);
    FAT_DIR* temp = &myFAT12.root;
    for(int i=0;i<path.size();i++){
        if(i==path.size()-1){
            FAT_FILE* file = temp->searchFileByName(path[i]);
            if(file== nullptr){
                myPrint("there is no such file! please check you path");
                lineSeperator();
                return;
            }
            else{
                myPrint(file->getContent());
                string s = file->getContent();
                lineSeperator();
            }
        }
        else{
            temp = temp->searchDirByName(path[i]);
            if(temp == nullptr){
                string wrong;
                wrong.append("there is no such dir:").append(path[i]).append("    please check your path");
                myPrint(wrong);
                lineSeperator();
                return;
            }
        }
    }
}
bool isAllSPACE(const string& s){
    if(s.empty()){
        return true;
    }
<<<<<<< HEAD
    for(char i : s){
        if(i!=' '){
=======
    for(int i=0;i<s.length();i++){
        if(s[i]!=' '){
>>>>>>> 40f25470eafcc492a0b8bf5b798af3544c5ec563
            return false;
        }
    }
    return true;
}
<<<<<<< HEAD
=======

>>>>>>> 40f25470eafcc492a0b8bf5b798af3544c5ec563
int main() {
    FAT12 myFAT12;
    myFAT12.setAddress("a.img");//设置img文件地址
    myFAT12.load();//加载img文件
    myFAT12.init();//读取img文件信息
    //以下是控制台程序
    myPrint("welcome to my fat12!");
    lineSeperator();
    myPrint("@rubisco");
    lineSeperator();
    myPrint("@2021-11-2");
    lineSeperator();
    myPrint("now,please type your opcode");
    lineSeperator();
    string opcode;
    start();
    string args;
    int i=0;
    getline(cin,args);
    for(;i<args.length();i++){
        if(args[i]==' '){
            opcode = args.substr(0,i);
            args = args.substr(i+1);
            break;
        }
    }
    if(opcode.empty()){
        opcode = args;
        args = "";
    }
    while(opcode!="exit"||!isAllSPACE(args)){
        if(opcode.length()<=1){
            myPrint("your opcode is too short.please type the right one!");
            lineSeperator();
        }
        else if(opcode=="ls"){
            showLS(args,myFAT12);
        }
        else if(opcode=="cat"){
            showCAT(args,myFAT12);
        }
        else{
            myPrint("there is no such opcode!please type the right one.");
            lineSeperator();
        }
        start();
        int idx=0;
        getline(cin,args);
        opcode = "";
        for(;idx<args.length();idx++){
            if(args[idx]==' '){
                opcode = args.substr(0,idx);
                args = args.substr(idx+1);
                break;
            }
        }
        if(opcode.empty()){
            opcode = args;
            args = "";
        }
    }
    myPrint("thank you for using! see you!");
    lineSeperator();
    return 0;
}

