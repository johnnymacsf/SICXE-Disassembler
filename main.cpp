#include <fstream>
#include <iostream>
#include <bitset>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

const static string ops[] = {
        "18", "58", "90", "40", "B4", "28",
        "88", "A0", "24", "64", "9C", "C4",
        "C0", "F4", "3C", "30", "34", "38",
        "48", "0", "68", "50", "70", "8",
        "6C", "74", "4", "D0", "20", "60",
        "98", "C8", "44", "D8", "AC", "4C",
        "A4", "A8", "F0", "EC", "C", "78",
        "54", "80", "D4", "14", "7C", "E8",
        "84", "10", "1C", "5C", "94", "B0",
        "E0", "F8", "2C", "B8", "DC"
};
const static string mnemonics[] = {
        "ADD", "ADDF", "ADDR", "AND", "CLEAR", "COMP",
        "COMPF", "COMPR", "DIV", "DIVF", "DIVR", "FIX",
        "FLOAT", "HIO", "J", "JEQ", "JGT", "JLT",
        "JSUB", "LDA", "LDB", "LDCH", "LDF", "LDL",
        "LDS", "LDT", "LDX", "LPS", "MUL", "MULF",
        "MULR", "NORM", "OR", "RD", "RMO", "RSUB",
        "SHIFTL", "SHIFTR", "SIO", "SSK", "STA", "STB",
        "STCH", "STF", "STI", "STL","STS", "STSW",
        "STT", "STX", "SUB", "SUBF", "SUBR", "SVC",
        "TD", "TIO", "TIX", "TIXR", "WD"
};

const static bool format2[] = {
        false,false,true,false,true,false,
        false,true,false,false,true,false,
        false,false,false,false,false,false,
        false,false,false,false,false,false,
        false,false,false,false,false,false,
        true,false,false,false,true,false,
        true,true,false,false,false,false,
        false,false,false,false,false,false,
        false,false,false,false,true,true,
        false,false,false,true,false
};

string LOCCTR;
string program_counter;
string base_content = "0000";
//outfile content
string progName;
string blockName;
string mnemonic;
string TA;
string objectCode;
string TAAM;
int i ;
vector<string> records;
bool isBased = false;

static int BinaryList[] = {0, 0, 0, 0, 0, 0, 0, 0};
std::map<std::string, std::vector<std::string>> symbols;
std::map<std::string, std::vector<std::string>> names;
bool isLiteral;
int update;

void ResettingBinaryList()
{
    for (int index = 0; index < 8; index++)
    {
        BinaryList[index] = 0;
    }
}

void DecToBinary(int decimalValue)
{
    ResettingBinaryList();

    for (int index = 7; (decimalValue > 0) || (index >= 0); index--)
    {
        BinaryList[index] = (decimalValue % 2);
        decimalValue = (decimalValue / 2);
    }
}

int getOpcode(string s, int position){
    string opcode = s.substr(position,2);
    ResettingBinaryList();

    DecToBinary(stoi(opcode, nullptr, 16));

    int maskFC[] = {1, 1, 1, 1, 1, 1, 0, 0};
    int opcodeBinary[8];
    for (int index = 0; index < 8; index++)
    {
        opcodeBinary[index] = (BinaryList[index]) & (maskFC[index]);
    }

    int totalL = (sizeof(ops) / sizeof(ops[0]));

    for (int index = 0; index < totalL; index++)
    {
        DecToBinary(stoi(ops[index], nullptr, 16));
        for (int indexTwo = 0; indexTwo < 8; indexTwo++)
        {
            if (!(opcodeBinary[indexTwo] == BinaryList[indexTwo]))
            {
                break;
            }
            if (indexTwo == 7)
            {
                return index;
            }
        }
    }
    return 0;
}
string register2(char s){
    string ret;
    if(s == '0'){
        ret = 'A';
    }else if(s == '1'){
        ret = 'X';
    }else if(s == '2'){
        ret = 'L';
    }else if(s == '3'){
        ret = 'B';
    }else if(s == '4'){
        ret = 'S';
    }else if(s == '5'){
        ret = 'T';
    }else if(s == '6'){
        ret = 'F';
    }else{
        ret = ret;
    }
    return ret;
}
string hexAdd(string LOC, string s){
    int a = stoi(LOC, nullptr, 16);
    int b = stoi(s, nullptr, 16);
    int result = a + b;
    char hex_string[20];
    sprintf(hex_string, "%X", result);

    string hex_result;
    hex_result = hex_string;

    if (hex_result.length() < 4) {
        int count = 4 - hex_result.length();
        while (count >= 1) {
            hex_result = "0" + hex_result;
            count--;
        }
    }
    return hex_result;
}

string convertToBinary(string s) {
    string hexS;
    string bin = "";
    for (char& _char : s) {
        if (_char == 'A') {
            bin += "1010";
        }
        else if (_char == 'B') {
            bin += "1011";
        }
        else if (_char == 'C') {
            bin += "1100";
        }
        else if (_char == 'D') {
            bin += "1101";
        }
        else if (_char == 'E') {
            bin += "1110";
        }
        else if (_char == 'F') {
            bin += "1111";
        }
        else {
            bin += bitset<4>(_char).to_string();
        }
    }
    return bin;
}

bool dashLine(const char* line)
{
    while (*line)
    {
        if (*line != '-')
            return false;
        ++line;
    }
    return true;
}

//parse through the second file
//it takes the table contents and places them in the respective hashmap
//there are two hashmaps, one of the symbols and one of the literals
//both hashmaps have a key of string (the address) and a vector of all the other information --- name, length, etc.
//the two hashmaps are then used in the code below
void parseSecondFile(const std::string& filename, std::map<std::string, std::vector<std::string>>& symbols, std::map<std::string, std::vector<std::string>>& names)
{
    char symbol[100], address[100], flag[100];
    char name[100], lit_const[100], length[100];
    bool inSymbols = false, inNames = false;

    char line[1000];
    FILE* file = fopen(filename.c_str(), "r");

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, "Symbol  Address Flags:") == 0 || strcmp(line, "Symbol  Value   Flags:") == 0)
        {
            inSymbols = true;
            inNames = false;
            continue;
        }

        if (strcmp(line, "Name    Lit_Const  Length Address:") == 0)
        {
            inSymbols = false;
            inNames = true;
            continue;
        }

        if (dashLine(line))
        {
            continue;
        }

        if (inSymbols && line[0] != '\0')
        {
            sscanf(line, "%s %s %s", symbol, address, flag);
            symbols[address] = { symbol, flag };

        }

        else if (inNames && line[0] != '\0')
        {
            sscanf(line, "%s %s %s %s", name, lit_const, length, address);
            names[address] = { name, lit_const, length };

        }
    }

    fclose(file);
}

string hexAddNeg(string dis){
    int a = stoi(program_counter, nullptr, 16);
    string displ;
    for(int j = 0; j < dis.length(); j++){
        if(dis[j] != 'F'){
            displ = displ + dis[j];
        }
    }
    string hexDisp = convertToBinary(displ);
    for(int j = 0; j < hexDisp.length(); j++){
        if(hexDisp[j] == '0'){
            hexDisp[j] = '1';
        }else{
            hexDisp[j] = '0';
        }
    }
    std::bitset<8> h1 (hexDisp);
    std::bitset<8> h2 ("0001");
    std::bitset<8> const m("1");
    std::bitset<8> h3;
    string ret;
    for(auto j = 0; j < h3.size(); ++j){
        std::bitset<8> const diff(((h1 >> j)&m).to_ullong() + ((h2 >> j)&m).to_ullong() + (h3 >> j).to_ullong());
        h3 ^= (diff ^ (h3 >> j)) << j;
    }
    std::string hRes = h3.to_string();
    hRes = "-" + hRes;
    int b = stoi(hRes, nullptr, 2);
    int c = a + b;
    if(c >= -2048 && c <= 2047){
        std::stringstream stream;
        stream << std::hex << c;
        std::string s4( stream.str());
        if(s4.length() < 4){
            int count = 4 - s4.length();
            while(count >= 1){
                s4 = "0" + s4;
                count--;
            }
        }
        for(int j = 0; j < s4.length(); j++){
            char c = s4[j];
            s4[j] = toupper(c);
        }
        ret = s4;
    }else{
        ret = "a";
    }
    return ret;
}

void parseHeader(string s, ofstream &outFile){
    progName = s.substr(1, 6);
    LOCCTR = s.substr(9, 4);
    outFile << LOCCTR << setw(8) << progName << setw(8) << "START" << setw(8) << "0" << endl;
}
void parseText(string s, ofstream &outFile){
    int diff;
    LOCCTR = s.substr(3,4);
    int start, end;
    start = 9;
    end = s.length();
    diff = end - start;
    while(start < end - 1){
        int index = getOpcode(s, start);
        int update;
        mnemonic = mnemonics[index];
        string format;
        bool isFormat2 = format2[index];

        if(isFormat2){
            format = "2";
            objectCode = s.substr(start,4);
            TA = register2(objectCode[2]);
            update = 4;
            program_counter = hexAdd(LOCCTR, "0002");
        }else{
            string flags = s.substr(start+1,2);
            DecToBinary(stoi(flags,nullptr,16));
            string displacement;
            if(BinaryList[7] == 0){
                format = "3";
                update = 6;
                objectCode = s.substr(start,6);
                displacement = objectCode.substr(3,3);
                if(displacement[0] == 'F'){
                    displacement = "F" + displacement;
                }else{
                    displacement = "0" + displacement;
                }
            }else{
                format = "4";
                update = 8;
                objectCode = s.substr(start,8);
                displacement = objectCode.substr(4,4);
                TAAM = "a";
            }
            if(diff <= 6 && records[i+1].at(0) == 'T'){
                program_counter = records[i+1].substr(3,4);
            }else{
                if(format == "4"){
                    program_counter = hexAdd(LOCCTR, "0004");
                }else{
                    program_counter = hexAdd(LOCCTR, "0003");
                }
            }

            if(BinaryList[5] == 0 && BinaryList[6] == 1){
                TAAM = "p";
            }else if(BinaryList[5] == 1 && BinaryList[6] == 0){
                TAAM = "b";
            }else if(BinaryList[5] == 0 && BinaryList[6] == 0){
                TAAM = "a";
            }
            if(TAAM == "p"){
                if(displacement[0] == 'F'){
                    string ret = hexAddNeg(displacement);
                    if(ret == "a"){
                        TAAM = "b";
                    }else{
                        TA = TA + ret;
                    }
                }else{
                    TA = TA + hexAdd(displacement, program_counter);
                }
            }
            if(TAAM == "b"){
                TA = TA + hexAdd(displacement, base_content);
            }
            if(TAAM == "a"){
                TA = TA + displacement;
            }
            if(mnemonic == "LDB"){
                isBased = true;
                base_content = TA;
            }
            if(BinaryList[2] == 0 && BinaryList[3] == 1){
                TA = "#" + TA;
            }else if(BinaryList[2] == 1 && BinaryList[3] == 0){
                TA = "@" + TA;
            }
            if(format == "4"){
                mnemonic = "+" + mnemonic;
            }
        }
        for (const auto& entry : symbols){
            std::string symbolAddress = entry.first;
            symbolAddress = symbolAddress.substr(2, 4);
            if(LOCCTR == symbolAddress){
                blockName = entry.second[0];
            }
        }
        for(const auto& entry : names){
            std::string literalAddress = entry.first;
            literalAddress = literalAddress.substr(2,4);
            if(LOCCTR == literalAddress){
                mnemonic = "BYTE";
                blockName = entry.second[0];
                TA = entry.second[1];
                int len = stoi(entry.second[2]);
                objectCode = entry.second[1].substr(2, len);
                if(entry.second[1].at(0) == 'X'){
                    program_counter = hexAdd(LOCCTR, to_string(len / 2));
                }else{
                    program_counter = hexAdd(LOCCTR, to_string(len));
                }
                update = len;
            }
        }
        outFile << LOCCTR << setw(8) << blockName << setw(8) << mnemonic << setw(12) << TA << setw(12) << objectCode << endl;

        if(isBased){
            outFile << setw(20) << "BASE" << setw(8) << base_content << endl;
        }
        blockName = "";
        isBased = false;
        LOCCTR = program_counter;
        start += update;
        diff = end - start;
        TA = "";
    }
}
void parseTextRecord(string filename){
    ifstream parseFile;
    ofstream outFile;
    parseFile.open(filename, ios::in);
    outFile.open("out2.lst");
    if(!parseFile.is_open()){
        cerr<< "File is not open!" << endl;
    }else{
        string s;
        while(getline(parseFile, s)){
            records.push_back(s);
        }
        parseFile.close();
        for(i = 0; i < records.size(); i++){
            string record = records[i];
            if(record.at(0) == 'H'){
                parseHeader(record, outFile);
            }
            if(record.at(0) == 'T'){
                parseText(record, outFile);
            }
        }
        outFile << setw(16) << "END" << setw(8) << progName << endl;
    }

    outFile.close();
}



int main(int argc, char** argv){
    //char* obj_file = argv[1];
    //char* sym_file = argv[2];
    parseSecondFile(argv[2], symbols, names);
    parseTextRecord(argv[1]);

    //parseTextRecord("test.obj");


    return 1;
}