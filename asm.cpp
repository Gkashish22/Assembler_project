/*

    MINI - PROJECT (CS-210 )

    TITLE - ASSEMBLER

    AUTHOR:
    Name - Kashish Garg
    Roll No - 2201CS36
    Email-ID - 2201cs36_kashish@iitp.ac.in

    Declaration of Authorship
    This asm.cpp is part of the miniproject of CS210 at the department of Computer Science and Engg, IIT Patna . 

    REQUIREMENTS - 

    TO Run This File You Need To Make 1 File named as "input.txt" which contains assembly code

    Assembler: (Requirement is input.txt should be present in same folder which contain assembly code) 
	(i) g++ asm.cpp -o asm
	(ii) ./asm

*/

#include<bits/stdc++.h>

using namespace std;
#define ll long long

fstream fin,fout,fout2;         // Input files
ofstream fo,lst,lg;             // Output files

           // Types of machine code
static const char coding_format[8] = "\\\'\'\"\t\f\b";   // Formating code
map<string, struct check_label> label_table; 
static map<string,pair<int,int>> operation_table;         // oper_code table
 multimap<int, string> warnings_checker;                 // Warnings table             
multimap<int, string> error_filler;                   // Errors table

list<struct line> normal_lines;                        // stroing normal_lines of code
list<struct line> label_lines;                    // Storing auxillary normal_lines


map<int,int> reserve_Data;                   // For storing reserved data
static const int pc_lower = 0x0;                // lower value of program_counter
static const int pc_upper = 0x3ff;              // upper value of program_counter
static const int min_offset = -0x00800000;      // Minimum value of offset
static const int offset_max = 0x007fffff;       // Maximum value of offset



static const int data_lower = 0x400;            // lower value of data
static const int address_end = 0xffff;          // maximum address it can reach

int program_counter, data_addr, line_cnt;
void optab(){          // storing value of each instruction with their type and oper_code
    operation_table["ldc"]    = {1,0}  ,operation_table["adc"]    = {1,1}  ,operation_table["ldl"]    = {1,2};
    operation_table["stl"]    = {1,3}  ,operation_table["ldnl"]   = {1,4}  ,operation_table["stnl"]   = {1,5}  ,operation_table["add"]    = {0,6};
    operation_table["sub"]    = {0,7}  ,operation_table["shl"]    = {0,8}  ,operation_table["shr"]    = {0,9}  ,operation_table["adj"]    = {1,10};
    operation_table["a2sp"]   = {0,11} ,operation_table["sp2a"]   = {0,12} ,operation_table["call"]   = {1,13} ,operation_table["return"] = {0,14};
    operation_table["brz"]    = {1,15} ,operation_table["brlz"]   = {1,16} ,operation_table["br"]     = {1,17} , operation_table["HALT"]  = {0,18};
}
int BinarytoDecimal(char binary[],int len, int i=0){
   if (i == len-1)

   return (binary[i] - '0');

   int temp=binary[i]-'0';
   temp = temp<<len-i-1;

   temp = temp + BinarytoDecimal(binary,len,i+1);
   return (temp);
}

struct line{                 // for storing normal_lines of code
    string code;                // memonic
    int line_no, address, hex_instr;          // line number, address, hex_instr
    line(string a, int next_addr, int c):code(a), line_no(next_addr), address(c), hex_instr(0) {}      // keeping hex_instr as 0 default
};
void HexToBin(string hexdec)
{
      //Skips "0x" if present at beggining of Hex string
    size_t i = (hexdec[1] == 'x' || hexdec[1] == 'X')? 2 : 0;
 
    while (hexdec[i]) {
 
        switch (hexdec[i]) {
        case '0':
            cout << "0000";
            break;
        case '1':
            cout << "0001";
            break;
        case '2':
            cout << "0010";
            break;
        case '3':
            cout << "0011";
            break;
        case '4':
            cout << "0100";
            break;
        case '5':
            cout << "0101";
            break;
        case '6':
            cout << "0110";
            break;
        case '7':
            cout << "0111";
            break;
        case '8':
            cout << "1000";
            break;
        case '9':
            cout << "1001";
            break;
        case 'A':
        case 'a':
            cout << "1010";
            break;
        case 'B':
        case 'b':
            cout << "1011";
            break;
        case 'C':
        case 'c':
            cout << "1100";
            break;
        case 'D':
        case 'd':
            cout << "1101";
            break;
        case 'E':
        case 'e':
            cout << "1110";
            break;
        case 'F':
        case 'f':
            cout << "1111";
            break;
        case '.':
            cout << ".";
            break;
        default:
            cout << "\nInvalid hexadecimal digit "
                 << hexdec[i];
        }
        i++;
    }
}
int machine_type = 0;   // storing machine type

struct check_label{
    private:            // these things are to be keep private
    string checker_string(const string &code){          // for checking string is valid or nor
 string s2="";
  if (!isalpha(code[0])) {
        return s2;
    }
    
    // Iterate through the string to check each character
    for (auto it = code.begin(); it != code.end(); ++it) {
        // Check if the character is a valid alphanumeric or space character
        
        if (*it <= 32 || !isalnum(*it)) {
            return s2;
        }
    }
    
    // If the string passes validation, return it
    return code;
}
    public:             // these things are to be public
    string line_code;        // check_label line_code
    int address, used_label, line_no;     // check_label address, check_label use count, check_label line number
    check_label(): line_code(""), address(-1), used_label(0), line_no(0) {}
    check_label(string code, int a): line_code(checker_string(code)), address(a), used_label(0), line_no(0) {}
};

void label_into_symtab(struct check_label& symbol){           // insreting label to symbol table if found else it is an error
    string labels = symbol.line_code;      // line_code of check_label
    auto it=label_table.find(labels);
    if(it!=label_table.end()) error_filler.insert({line_cnt, "label line_code exists already"});      // duplicate check_label found
    else label_table[labels] = symbol;      // storing check_label in symbol table
}



bool validity_number(string code) {
    auto iterator = code.begin();
    
    // Ignore leading '+' or '-'
    if (*iterator == '-'||*iterator=='+') ++iterator;
    
    // If the string is empty, return false
    if (iterator == code.end()) return false;
    
    // If the operand character is '0'
    if (*iterator != '0') {
         for (auto subIterator = iterator; subIterator != code.end(); subIterator++) {
        if (!isdigit(*subIterator)) return false; // Not a digit
    }
       
    }
    else{
    // Check the remaining characters, ensuring they are all digits
    ++iterator;
        
        // If the next character is 'x' or 'X', it'code a hexadecimal number
        if (iterator == code.end()) return true;
        if (*iterator == 'x' || *iterator == 'X') {
            ++iterator;
            // Check if the following characters are valid hexadecimal digits
            while (iterator != code.end()) {
                if ((*iterator >= '0' && *iterator <= '9') || 
                    (*iterator >= 'a' && *iterator <= 'f') || 
                    (*iterator >= 'A' && *iterator <= 'F')) ++iterator;
                else return false; // Not a valid hexadecimal digit
            }
            return true; // All characters are valid hexadecimal digits
        }
        else {
            // Check if the following characters are valid octal digits
            while (iterator != code.end()) {
                if (*iterator >= '0' && *iterator <= '7') ++iterator; // Valid octal digit
                else return false; // Not a valid octal digit
            }
        }
    }
    
    return true; // All characters are valid digits
}
void too_few(int line){
 error_filler.insert({line,"Too few operands"});
}

int str_to_int(string &num){            // Convert string to int
    istringstream iss(num);
    int a = 0;
    auto it = num.begin();
    if(*it=='+' || *it=='-') ++it;              // skiping if operand char is +,-

    if(*it=='0') { ++it;
        if(it==num.end()) return 0;
        if(*it=='x' || *it=='X') iss>>hex>>a;
        else iss>>oct>>a;}
    else{
     iss>>a;  
    }
    iss>>dec;
    return a;       // returning number
}

void increase_data(){           // Increase value of data adrress
    ++data_addr;
}

void data_re(int a){            // storing value of a to reserved data
    reserve_Data[data_addr] = a;
}
void too_many(int line){
    error_filler.insert({line,"Too many operands"});
}
void number_not_found(int line){
     error_filler.insert({line,"Expecting a number"});
}
void label_not_found(int line){
     error_filler.insert({line,"Invalid Label"});
}
void preceding_label(int line){
     warnings_checker.insert({line,"Preceding label missing"});
}
void analyse(string line_code){                  // analysing the final instruction we got from pass1
    int ind=line_code.find(':');         // FInding if a string contaning : for a check_label
    if(ind >=line_code.size()){
        istringstream iss(line_code);
        string mnemonic;                      // Finding the memonic
        iss>>mnemonic;
        if(mnemonic=="data"){
             int error = 0;                 // If memonic is data
            string operand,second;
           
            iss>>operand;                  // Reading the next number we will get
            if(operand.size()==0){           // If there is no number then it an error
              too_few(line_cnt);
                return;
            }
            mnemonic=operand;
            if(validity_number(mnemonic)!=true){
                number_not_found(line_cnt);
                error =1;
            }
            iss>>second;              // reading one second number which should be empty
            if(second.size()>0){              // Ifwe found numbers then it error
                too_many(line_cnt);
                error =1;
            }
            if(error==0){           // if error is other than 0
                int a = str_to_int(mnemonic);         // calculating integer from this string
               
                struct line aux(line_code, line_cnt, data_addr);
                aux.hex_instr = a;
                label_lines.push_back(aux);  // Storing it in auxillary normal_lines
                 data_re(a);     
                data_addr++;           // increasing data address
            }
            return;
        }
        else if(mnemonic=="SET"){                 // If memonic is set
           preceding_label(line_cnt);          // Printing warnings_checker
           
            int error = 0;
             string operand,second;
            iss>>operand;              // Reading a integer
            if(operand.size()==0){               // if integer is present or not
                  too_few(line_cnt);
                return;
            }
            mnemonic = operand;
            if(validity_number(mnemonic)!=true){                // if number is a valid number 
                number_not_found(line_cnt);
                error=1;
            }
            iss>>second;
            if(second.size()>0){                      // we should not expect any further operand
               too_many(line_cnt);
                error = 1;
            }
            if(error==0){                // if error is 0
                int a = str_to_int(mnemonic);
                struct line aux(line_code, line_cnt, data_addr);
                aux.hex_instr = a;
                label_lines.push_back(aux);       // Storing it in auxillary normal_lines
            }
            return;
        }
        else {
            struct line l_t("",0,0);
            l_t.code=line_code;
            l_t.line_no=line_cnt;
            l_t.address=program_counter;
           program_counter++;               // increasing program_counter
            normal_lines.push_back(l_t);             // storing line in normal_lines
            return;
        }
    }
    else{
        string temp2="";
        string temp1="";
        for(int j=ind+1;j<line_code.size();j++){
                   temp2=temp2+line_code[j];
        }
          for(int j=0;j<ind;j++){
                   temp1=temp1+line_code[j];
        }
        istringstream iss(temp2);               // Reading string temp2;
        string mnemonic;
        struct check_label symbol(temp1, -1);
        iss>>mnemonic;
        if(mnemonic=="data"){  
             int error = 0;               // if a string is data
            if(symbol.line_code.empty()) {label_not_found(line_cnt);}     //  if line_code is empty then it is a error
           
            string operand,second;
            iss>>operand;
            if(operand.size()==0){                   // if number is empty then it is error
                  too_few(line_cnt);    
                return;
            }
            mnemonic=operand;
            if(validity_number(mnemonic)!=true){           // If a number is valid or not
                number_not_found(line_cnt);
                error=1;
            }
            string num = mnemonic;
            iss>>second;
            if(second.size()){                  // we donot expect other number so it should br empty
               too_many(line_cnt);
                error=1;
            }
            if(error==0){
                int value = str_to_int(num);
                            
                symbol.address = data_addr;
                symbol.line_no = line_cnt;
                label_into_symtab(symbol);          // inserting it in symbol tabel
                struct line aux(line_code, line_cnt, data_addr);
                aux.hex_instr = value;
                label_lines.push_back(aux);               // storing in auxillary normal_lines
                 data_re(value);// reserving the data
                increase_data();                    // increasing data address
            }
            return;
        }
        else if(mnemonic == "SET"){               // if memonic is set
            int error=0;
            string operand;
            if( symbol.line_code.empty() ){               // line_code of check_label should not be zero
               label_not_found(line_cnt);
                error=1;
            }
            iss>>operand;
            if(operand.size()==0){                   // if string is empty then error
                too_few(line_cnt);
                return;
            }
            mnemonic = operand;
            if(validity_number(mnemonic)!=true){            // if string is not a valid number
                 number_not_found(line_cnt);
            }
            string num = mnemonic;
            string second;
            iss>>second;
            if(second.size()){                      // we should not expect any further operand
                too_many(line_cnt);
                error = 1;
            }
            if(error==0){                        // if there is no error
                int a = str_to_int(num);            // converting string to integer
                symbol.address = a;
                symbol.line_no = line_cnt;
                label_into_symtab(symbol);                  // inserting check_label into symbol table
                struct line aux(line_code, line_cnt, program_counter);
                aux.hex_instr = a;                       // encoded form of opearnd of SET
                label_lines.push_back(aux);                   // Storing in auxllary normal_lines
            }
            return;
        }
        else{
            int error = 0;
            string s4=symbol.line_code;
            if(s4.size()==0){                     // if line_code is empty then error
              label_not_found(line_cnt);
                error=1;
            }
            symbol.address = program_counter;
            symbol.line_no = line_cnt;
            if(error==0){               // if there is no error
                label_into_symtab(symbol);
                string s3=symbol.line_code+":";
                struct line aux(s3, line_cnt, program_counter);
                label_lines.push_back(aux);           // storing line in auxillry line
            }
            if(mnemonic.size()){
                struct line instr(temp2, line_cnt, program_counter);
                program_counter++;
                normal_lines.push_back(instr);              // storing normal_lines
            }
            return;
        }
    }
    return;
}

int zero_type(const string &code, const int& oper_code, const int& line_no){
    istringstream iss(code);
    string mnemonic;
    iss>>mnemonic;     // 'dump' mnemonic
    string operand_checker;
    iss>>operand_checker;
    if(operand_checker.size()==0)   // second operands than required so error
    { return oper_code;
      
    }
    else{
         too_many(line_no);
        return 0xffffffff;              // returning the case that it is error
    }
   
}



int first_type(const string& code, const int& oper_code, const int& line_no, const int& program_counter){
    istringstream iss(code);
    string mnemonic;
    iss>>mnemonic;     // 'dump' mnemonic
    string operand, second;
    iss>>operand;
    if(operand.size()==0)    // no operand - generate too few operands error
    {
         too_few(line_no);
        return 0xffffffff;
    }
    mnemonic = operand;
    iss>>second;
    if(second.size()!=0)   // second than 1 operand - generate too many operands error
    {
        too_many(line_no);
        return 0xffffffff;
    }
    if(validity_number(mnemonic)!=false)   // a numeric operand
    {
        int a = str_to_int(mnemonic);
        a = a<<8;
        a = a + oper_code;
   int final=a;
        return final;
    }
    else        // either a check_label or an invalid operand
    {
        struct check_label data_give(mnemonic, 0);
        if(data_give.line_code.size()!=0)    // not a check_label
        {
          auto label_find = label_table.find(mnemonic);
        if(label_find==label_table.end())    // check_label not in symbol table
        {
            error_filler.insert({line_no, "No such label line_code found"});
            return 0xffffffff;
        }
        int z=label_find->second.used_label+1;
        label_find->second.used_label=z;

        int offset;
        int next_addr;

       if (oper_code == 13 || oper_code == 15 || oper_code == 16 || oper_code == 17) {
    next_addr = label_find->second.address - (program_counter + 1);
    next_addr = next_addr << 8;
    next_addr = next_addr + oper_code;
    return next_addr;  // returning final address
} else {
    next_addr = label_find->second.address;
    next_addr = next_addr << 8;
    next_addr = next_addr + oper_code;
    return next_addr;  // returning final address
} 
        }
        else{
               error_filler.insert({line_no, "Invalid operand"});
            return 0xffffffff;
        }
        

    }
}

void pass1(){           // First pass of Assembler
    string line_code;
    while( getline(fin, line_code) ){            // reading a string from input file
        string data_give;
        int if_begin_line=0;
        ++line_cnt;
        for(int i=0; i<line_code.size();i++){ 
            if(line_code[i]==';')break;           // for loop for removing intial space and content after ; as oit has to be ignored
            if( int(line_code[i])<=32 && if_begin_line==0 ) continue;
            else {
                if_begin_line++;
                data_give.push_back(line_code[i]);
            }
        }
        if(data_give.size()!=0) analyse(data_give);      // we got an instruction and applying analyses
    }
}
void Unknown_mnemonics(int line){
 error_filler.insert({line, "Unknown mnemonic"});
}
void pass2(){           // Second pass of Assembler
// Iterate over each line in the 'normal_lines' container

    for( auto it=normal_lines.begin();it!=normal_lines.end(); ){           // Iterating till we did not complete normal_lines
        string code =  it->code;
        istringstream iss(code);
        string mnemonic;
        iss>>mnemonic;              // Finding memonics
        auto mnemonic_found = operation_table.find(mnemonic);             // finding pointer of mnemonic
        if(mnemonic_found==operation_table.end()){                    // If we donot find mnemonic in op_tab then error
           Unknown_mnemonics(it->line_no);
            continue;
        }
            int oper_code = mnemonic_found->second.second;         // fining oper_code
        int operand_type = mnemonic_found->second.first;          // finding i type
    
        if(operand_type==0){                  // if i type is 0
            int res = zero_type(code, oper_code, it->line_no);
            if( res!=0xffffffff ) it->hex_instr = res;
        }
        else{
            int res = first_type(code, oper_code, it->line_no, it->address);
            if(res!=0xffffffff) it->hex_instr = res;
        }
        it++;               // going to next instruction
    }


}

void check_unused_label(){                // Generating Warning
    auto it = label_table.begin();
    while(it!=label_table.end()){ 
        int use=it->second.used_label;       // iterating over sumtab
        int line_used=it->second.line_no;
        if(use==0) warnings_checker.insert({line_used, "Label not in use"});
        it++;
    }
}

bool label_check(const string& code){       // checking for check_label
    auto it = code.begin();
    while(it!=code.end()){
        if(*it==':') break;
        it++;             // we will go till : for finding check_label is present or not
    }
    if(it==code.end()) return false;       // check_label is not there
    it++;    
   while(it!=code.end()){
        if(*it<=32) it++; 
        else return false;
    }
    return true;
}

void generate_listing_file() {              // Creating listing file
    // Check if there are no error before creating the file
if (error_filler.empty()) {
    // Write the header to the file
    lst << setw(10) << "MemAddress" << setw(10) << "Encoding" << '\t' << "Lines_Code" << endl << endl << uppercase << setfill('0');
    auto it_main = normal_lines.begin(); // Iterator for main normal_lines
    auto it_aux = label_lines.begin(); // Iterator for auxiliary normal_lines
     // Write main and auxiliary normal_lines to the file
    while (it_main != normal_lines.end() && it_aux != label_lines.end()) {
        if (it_aux->line_no == it_main->line_no) {
            // Write main and auxiliary normal_lines together if they have the same line number
            lst << setw(10) << it_main->address << '\t' << "0x" << setw(8) << hex << it_main->hex_instr << '\t' << dec << it_aux->code << it_main->code << endl;
            it_main++;
            it_aux++;
        } else if (it_main->line_no > it_aux->line_no) {
            // Write auxiliary line if it has a higher line number than the current main line
            lst << setw(10) << it_aux->address << '\t';
            if (label_check(it_aux->code) != true) lst << "0x" << setw(8) << hex << it_aux->hex_instr << '\t' << dec; // Write hex_instr if not a label
            else lst << "        \t"; // Otherwise, leave hex_instr field empty
            lst << it_aux->code << endl;
            it_aux++;
        } else {
            // Write main line if it has a higher line number than the current auxiliary line
            lst << setw(10) << it_main->address << '\t' << "0x" << setw(8) << hex << it_main->hex_instr << '\t' << dec << it_main->code << endl;
            it_main++;
        }
    }

    // Write remaining main normal_lines
    while (it_main != normal_lines.end()) {
        lst << setw(10) << it_main->address << '\t' << "0x" << setw(8) << hex << it_main->hex_instr << '\t' << dec << it_main->code << endl;
        it_main++;
    }

    // Write remaining auxiliary normal_lines
    while(it_aux!=label_lines.end()) {
        lst << setw(10) << it_aux->address << '\t';
        if (label_check(it_aux->code) != true) lst << "0x" << setw(8) << hex << it_aux->hex_instr << '\t' << dec; // Write hex_instr if not a label
            else lst << "        \t"; // Otherwise, leave hex_instr field empty
        lst << it_aux->code << endl;
        it_aux++;
    }
    
    lst << setfill(' ') << nouppercase; // Reset formatting flags
    lst.close(); // Close the file
}

    // ELSE, some error in source program - leave the previous listing file as it is
}

void generate_log_file(){               // Creating log file
 // Writing warnings_checker to the file if there are any
if (warnings_checker.size()) {
    lg << "Warnings -" << endl;
    for (auto wt = warnings_checker.begin(); wt != warnings_checker.end(); ++wt) {
        lg << "Line " << wt->first << ": " << wt->second << endl;
    }
    lg << endl;
}

// Writing error_filler to the file if there are any
if (error_filler.size()) {
    lg << "Errors -" << endl;
    for (auto et = error_filler.begin(); et != error_filler.end(); ++et) {
        lg << "Line " << et->first << ": " << et->second << endl;
    }
    lg << endl;
}

lg.close(); // Closing the file stream
           // Closing file
}

void print_bytes(ostream& os, int a){           // Function for printing bytes
    char *ptr = reinterpret_cast<char*>(&a); // Pointer to the integer'code memory representation

// Check machine endianness
if (machine_type == 0) {
    // Reverse the byte order for little-endian machines
    ptr += sizeof(int) - 1; // Move pointer to the last byte
    for (int i = 0; i < sizeof(int); ++i) {
        os.write(ptr--, 1); // Write each byte in reverse order
    }
} else {
    // For big-endian machines, write bytes in their natural order
    for (int i = 0; i < sizeof(int); ++i) {
        os.write(ptr++, 1); // Write each byte in order
    }
}

}

void generate_object_file(){            // Generating object file
    // Define newline character
const char newline = '\n';

// Write coding_format to the file
const char* ptr = coding_format;
for (int i = 0; i < 8; ++i) {
    fo.write(coding_format + i, 1);
}

// Write newline character to the file
fo.write(&newline, 1);

// Get text segment size and write it to the file
int text_size = program_counter;
print_bytes(fo, text_size);
fo.write(&newline, 1);

// Write text segment operands to the file
auto it = normal_lines.begin();
for (int i = 0; i < text_size; ++i) {
      print_bytes(fo, it->hex_instr);
    ++it;
}

// Write two newline characters to the file
fo.write(&newline, 1);
fo.write(&newline, 1);

// Get data segment size and write it to the file
int data_size = reserve_Data.size();
print_bytes(fo, data_size);
fo.write(&newline, 1);

// Write data segment to the file
auto dt = reserve_Data.begin();
for (int i = 0; i < data_size; ++i) {
print_bytes(fo, dt->second);
    ++dt;
    
}

// Close the file
fo.close();


}

void reset(){                   // Reseting all constraints
   
    error_filler.clear();
    warnings_checker.clear();
       data_addr = data_lower;
       program_counter = pc_lower;
    line_cnt=0; 
     operation_table.clear();   
}
void exit(){
    exit(0);
}
void open_file(){               // Opening files
    fin.open("input.txt",ios::in);
    if(!fin){                       // exiting if there is error in opening file
        cout<<"Error :";
        cout<<"Give Input file\n";
       exit();
    }
    fo.open("object_file.txt",ios::binary|ios::out);
    lg.open("log_file.txt",ios::out);
    lst.open("listing_file.txt",ios::out);
}
int main(){

    reset();            // reseting
    open_file();        // opening files
    optab();       // loading opcodes
    pass1();            // pass1 of Assembler
    pass2();            // pass2 of Assembler
 check_unused_label();
    generate_listing_file();
    generate_log_file();
    generate_object_file();
    fin.close();                // Closing file
    fo.close();
    lst.close();
    lg.close();

}