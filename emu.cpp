
/*

    MINI - PROJECT ( CS-210 )

    TITLE - EMULATOR

    AUTHOR:
    Name - Kashish Garg
    Roll No - 2201CS36
    Email_id- 2201cs36_kashish@iitp.ac.in
    Declaration of Authorship
    This emu.cpp file, is part of the miniproject of CS210 at the department of Computer Science and Engg, IIT Patna .

    REQUIREMENTS -

    To Run This File you need Assembler which generate a object file which is a input to this file

    Emulator: (Requirement is object file named as object_file.txt which is produced by asm.cpp)
    (i) g++ emu.cpp -o emu
    (ii) ./emu

*/

#include <bits/stdc++.h>
#include <iostream>
#include <iomanip>

#include <string>
#include <cctype>

#include <utility>
#include <map>
#include <set>
#include <list>

#include <fstream>
#include <ostream>
#include <sstream>

using std::cin;
using std::cout;

using std::list;
using std::map;
using std::multimap;
using std::pair;
using std::set;
using std::string;

using std::ifstream;
using std::istringstream;
using std::ofstream;
using std::ostream;

using std::dec;
using std::endl;
using std::ends;
using std::flush;
using std::hex;
using std::noshowbase;
using std::nounitbuf;
using std::nouppercase;
using std::oct;
using std::setfill;
using std::setw;
using std::showbase;
using std::unitbuf;
using std::uppercase;
using namespace std;

string machine_file;
enum endian
{
    lil_endian,
    big_endian
}; // For machine type

fstream fi; // input file

static map<int, pair<int, string>> op_decoded; // Decoder table

static int data_lower = 0x400; // Lower value of data

static int end_address = 0xffff; // Last address

static int pc_min = 0x0; // Minimum value of PC

static int pc_max = 0x3ff; // Maximum value of PC

static int total_memory = end_address + 1; // Total memory

static int long long max_instruction_count = 0xfffff; // maximum instruction count

int text_size;
int data_size;
string error_Cause;
bool run_code;
bool error_check;
long long int instr_cnt;
int A, B;
int PC, SP;
int memory_space[100010];
int data_upper;

static char format_code[8] = "\\\'\'\"\t\f\b"; // For formitting code

void decoder_instruction()
{ // Loading op_decoded table
    op_decoded[0] = {1, "ldc"}, op_decoded[1] = {1, "adc"}, op_decoded[2] = {1, "ldl"}, op_decoded[3] = {1, "stl"};
    op_decoded[4] = {1, "ldnl"}, op_decoded[5] = {1, "stnl"}, op_decoded[6] = {0, "add"}, op_decoded[7] = {0, "sub"};
    op_decoded[8] = {0, "shl"}, op_decoded[9] = {0, "shr"}, op_decoded[10] = {1, "adj"}, op_decoded[11] = {0, "a2sp"};
    op_decoded[12] = {0, "sp2a"}, op_decoded[13] = {1, "call"}, op_decoded[14] = {0, "return"}, op_decoded[15] = {1, "brz"};
    op_decoded[16] = {1, "brlz"}, op_decoded[17] = {1, "br"}, op_decoded[18] = {0, "HALT"};
}

static endian machine_type = lil_endian; // defining machine type

int reverse_decode_to_int(fstream &fi)
{ // Getting integer on bases of machine type
    int res = 0;
    char *ptr = (char *)&res;
    char byte = 0;
    if (machine_type == big_endian) // just read byte and put it
    {
        fi.read(&byte, 1);
        *ptr = byte;
        ptr++;

        fi.read(&byte, 1);
        *ptr = byte;
        ptr++;

        fi.read(&byte, 1);
        *ptr = byte;
        ptr++;

        fi.read(&byte, 1);
        *ptr = byte;
    }

    else // read and put in reverse order
    {
        ptr += 3;

        fi.read(&byte, 1);
        *ptr = byte;
        ptr--;

        fi.read(&byte, 1);
        *ptr = byte;
        ptr--;

        fi.read(&byte, 1);
        *ptr = byte;
        ptr--;

        fi.read(&byte, 1);
        *ptr = byte;
    }
    return res;
}
void Unknown()
{
    cout << "Object file format not known" << endl;
}
bool loader()
{                                        // Loader to load Emulator
    fi.open("object_file.txt", ios::in); // Opening file object_file
    if (!fi)
        return false; // If file does not open
    char aux[8] = "\0";
    char new_line;

    for (int i = 0; i < 8; ++i)
        fi.read(aux + i, 1);
    int i = 0;
    while (i < 8)
    {
        if (aux[i] != format_code[i])
        { // error found -> Unknown object file
            Unknown();
            return false;
        }
        i++;
    }
    fi.read(&new_line, 1);
    if (new_line != '\n')
    {
        // Error found -> Unknown object file
        Unknown();
        return false;
    }
    text_size = reverse_decode_to_int(fi); // Geting integer from a file
    fi.read(&new_line, 1);
    i = 0;
    while (i < text_size)
    {
        memory_space[i] = reverse_decode_to_int(fi);
        i++;
    } //  storing the integers
    fi.read(&new_line, 1);
    fi.read(&new_line, 1);
    data_size = reverse_decode_to_int(fi); // Getting integer from a file
    fi.read(&new_line, 1);

    data_upper = data_lower + data_size - 1; // Updating value of upper data
    for (int i = 0; i < data_size; ++i)
        memory_space[i + data_lower] = reverse_decode_to_int(fi);
    PC = 0x00000000, A = 0x00000000, B = 0x00000000, SP = 0xff00; // Storing values in PC, A, B, SP

    fi.close();
    return true;
}

int status_of_program()
{
    if (run_code == true)
        return 0; // run_code execution          // Getting situation of program
    if (error_check == true)
        return -1;
    return 1;
}

string decode_oper(int a)
{                                 // Reverse decoding for a integer a
    int opcode = (a << 24) >> 24; // Calculating opcode;
    ostringstream oss;

    auto mt = op_decoded.find(opcode);  // Finding pointer of opcode in op_decoded
    string mnem = mt->second.second; // storing the mnemonic
    oss << mnem;
    if (mt->second.first == 1)
    {
        int tmp = (a & 0xffffff00);
        tmp = tmp / 256;
        oss << " " << tmp;
    }
    return oss.str();
}

void disassemble(ostream &os)
{ // Function for disaasembling
    int i = 0;
    while (i < text_size)
    {
        string tmp = decode_oper(memory_space[i]); // calling decode_oper
        os << "PC = " << setw(10) << setfill('0') << i << setfill(' ') << "   "
           << "Memory Content = "
           << "0x" << setw(8) << setfill('0') << hex << memory_space[i] << dec << setfill(' ') << "   ";
        cout << " " << tmp << endl;
        i++;
    }
}

void memory_dump(ostream &os)
{ // Function for dumping memory
    os << hex << showbase << "Text Segment :-\n\tsize = " << text_size << '\n';
    for (int i = pc_min; i < text_size; i++)
    {
        os << setw(8) << setfill('0') << i << ": " << memory_space[i] << '\n';
    }

    // Printing Data Segment information
    os << "\n\n"
       << "Data Segment :-\n\tsize = " << data_size << '\n';
    for (int i = 0; i < data_size; i++)
    {
        os << setw(8) << setfill('0') << i + data_lower << ": " << memory_space[i + data_lower] << "  (In decimal: " << dec << memory_space[i + data_lower] << hex << ")\n";
    }
    os << '\n'
       << noshowbase << dec; // Reset stream settings
}

string current_state()
{ // Function for finding current status
    ostringstream oss;
    oss << setw(11) << "PC = " << PC << "   " << setw(11) << "SP = " << SP << "   " << setw(11) << "A = " << A << "   " << setw(11) << "B = " << B << "   ";
    oss << "Memory Content = "
        << "0x" << setw(8) << setfill('0') << hex << memory_space[PC] << dec << setfill(' ') << "   "
        << " " << decode_oper(memory_space[PC]);
    return oss.str();
}
string sp_error()
{string s3=" SP goes out of segment ";
    return s3 ;
}
string out_of_text(){
    string s5="Program tried to access text segement memory area";
    return s5;
}
bool check_memory_access(int a)
{ // Function to accessing memory
    string tmp;
    if (a <= -1)
    { // a should be positive
     tmp=out_of_text();
        error_check = true;
       
    } // a should be less than end_address
    else if (a > end_address)
    { tmp=out_of_text();
        error_check = true;
       
    }
    if (a <= pc_max)
    { // a should be less than upper value of PC
    tmp=sp_error();
        error_check = true;
       
    }
    if (error_check == false)
        return true; // If no error then return true
    PC--;
    string tmp2 = current_state(); // Finding current state
    PC++;
    error_Cause = tmp + " when executing " + tmp2; // storing error in error_Cause
    return false;
}

void operation()
{
    error_check = true;
    PC -= 1;
    string tmp2 = current_state(); // Checking the current status
        error_Cause = sp_error();             
    error_Cause = error_Cause+" when executing " + tmp2; // storing error_check in falt_cause
    PC++;
}

bool check_SP(int a)
{ // Checking the SP is in range or not
    if (a > end_address)
    { // value of a should be less than last value of address
        operation();
        return false;
    }
    else if (a <= data_upper)
    { // value should be greater than uppper value of data
        operation();
        return false;
    }
    return true;
}
string pc_error(){
    string s4="Pc goes out of text segment";
    return s4;
}
void operation_1()
{
    error_check = true;
    PC--;
    string tmp2 = current_state(); // getting current status
    PC++;
    error_Cause=pc_error();
    error_Cause = error_Cause + " when executing " + tmp2; // Storing error in falt_cause
}

bool check_PC(int a)
{ // Checking the PC is in range or not
int f=0;
    if (a < 0)
    {
        operation_1();
      f=1;
    }
    else if (a > pc_max)
    {
        operation_1();
       f=1;
    }
    if(f==0)
    return true;
    else return false;
}

string execute()
{
    string s2 = "";
    if (error_check == true)
        return error_Cause;
    if (run_code == true)
        return s2;
    if (text_size == 0)
    {
        run_code = true;
        string s3 = "No text segement ";
        return s3;
    }
    
    int mach_code = memory_space[PC]; // get current machine code
    int oper_code = (mach_code << 24) >> 24;
string res = current_state();
    int opcode = oper_code;
    int operand = (mach_code & static_cast<int>(0xffffff00)); // cast required for operations to work correctly work correctly
    operand /= 256;

    ++PC; // PC incremented to point to next instruction

    check_PC(PC);
    if (PC > pc_max)
    {
        error_check = true;
        error_Cause = "PC goes out of text segement  as upper limit for text segment crossed; no HALT encountered";
        return error_Cause;
    }

    switch (opcode)
    {
    case 0: // ldc value
        B = A;
        A = operand;
        break;
    case 1: // adc value
        A = A + operand;
        break;
    case 2: // ldl value
        check_memory_access(SP + operand);
        if (error_check)
            return error_Cause;
        B = A;
        A = memory_space[SP + operand];
        break;
    case 3: // stl value
        check_memory_access(SP + operand);
        if (error_check)
            return error_Cause;
        memory_space[SP + operand] = A;
        A = B;
        break;
    case 4: // ldnl offset
        check_memory_access(A + operand);
        if (error_check)
            return error_Cause;
        A = memory_space[A + operand];
        break;
    case 5: // stnl offset
        check_memory_access(A + operand);
        if (error_check)
            return error_Cause;
        memory_space[A + operand] = B;
        break;
    case 6: // add
        A = B + A;
        break;
    case 7: // sub
        A = B - A;
        break;
    case 8: // shl
        A = B << A;
        break;
    case 9: // shr
        A = B >> A;
        break;
    case 10: // adj value
        check_SP(SP + operand);
        if (error_check)
            return error_Cause;
        SP = SP + operand;
        break;
    case 11: // a2sp
        check_SP(A);
        if (error_check)
            return error_Cause;
        SP = A;
        A = B;
        break;
    case 12: // sp2a
        B = A;
        A = SP;
        break;
    case 13: // call offset
        check_PC(PC + operand);
        if (error_check)
            return error_Cause;
        B = A;
        A = PC;
        PC = PC + operand;
        break;
    case 14: // return
        check_PC(A);
        if (error_check)
            return error_Cause;
        PC = A;
        A = B;
        break;
    case 15: // brz offset
        if (A == 0)
        {
            check_PC(PC + operand);
            if (error_check)
                return error_Cause;
            PC = PC + operand;
        }
        break;
    case 16: // brlz offset
        if (A < 0)
        {
            check_PC(PC + operand);
            if (error_check)
                return error_Cause;
            PC = PC + operand;
        }
        break;
    case 17: // br offset
        check_PC(PC + operand);
        if (error_check)
            return error_Cause;
        PC = PC + operand;
        break;
    case 18: // HALT
        run_code = true;
        break;
    default:
        error_check = true;
        error_Cause = "Opcode is not valid";
        return error_Cause;
    }

    ++instr_cnt;

    if (instr_cnt >= max_instruction_count) // looped back to 0 -> limit crossed
    {

        // indiacte program took too long to finish
        ostringstream oss;
        error_check = true;
        oss << "Program took too long to finish";
        oss << '\n';
        oss << instr_cnt << " instructions executed till now\n";

        error_Cause = oss.str();

        return error_Cause;
    }

    return res;
}

string print_options()
{ // These are diffrent printing option we have provided to user
    string options;
    options += "Options :-\n";
    options += "\tq - Show current architectural state\n";
    options += "\tt - Execute one instruction with trace\n";
    options += "\ts - Execute all instructions with trace\n";
    options += "\td - Show memory dump\n";
    options += "\tn - Give number of instructions to execute \n";
    options += "\ta - Execute all instructions without trace\n";

    options += "\tc - Display number of instructions executed so far\n";
    options += "\tu - Disassemble\n";
    options += "\tk - Show memory dump after instruction\n";
    options += "\tx - Quit the emulator\n\n";
    return options;
}

int long long instructions_executed()
{
    return instr_cnt;
}
int failure()
{
    return 1;
}
void terminated()
{
    cout << "Program Terminated" << endl;
}
void successfull()
{
    cout << "Success, Object file loaded properly" << endl;
}
int main()
{
    decoder_instruction(); // loading op_decoded table
    cout << unitbuf;
    cin.tie(&cout);
    int loading = loader(); // calling loader function to start emulator
    if (loading == 0)
    { // If loading is not done correctly then error
        cout << "Object file not loaded properly" << endl;
        terminated();
        failure();
    }
    else
        successfull();
    string options = print_options();
    cout << options;
    cout << endl; // Printing printing options
    char opt;     // interact with the user
    do
    {
        cin >> opt;
        switch (opt)
        {
        case 'a': // execute all instructions in one go
            if (status_of_program() == 0)
            {
                cout << "Program finished execution" << endl
                     << endl;
                break;
            }
            if (status_of_program() == -1)
            { // program in error state
                cout << error_Cause << endl
                     << endl;
                break;
            }
            while(true)
            {
                if (status_of_program() == 1)
                    cout << execute() << endl;
                else
                    break;
            }
            if (status_of_program() == -1)
            { // program in error_check state after completing execution
                cout << error_Cause << endl
                     << endl;
                break;
            }
            cout << "Total instructions executed = " << instructions_executed() << endl
                 << endl;
            break;
        case 't': // execute a single instruction with trace
            if (status_of_program() == -1)
            {
                cout << error_Cause << endl
                     << endl;
                break;
            }
            if (status_of_program() == 0)
            {
                cout << "Program finished execution" << endl
                     << endl;
                break;
            }

            cout << execute() << endl;
            cout << endl;
            break;
        case 's': // execute all instrutions with trace
            if (status_of_program() == 0)
            {
                cout << "Program finished execution" << endl
                     << endl;
                break;
            }
            if (status_of_program() == -1)
            { // program in error_check state
                cout << error_Cause << endl
                     << endl;
                break;
            }
          while(true)
            {
                if (status_of_program() == 1)
                    cout << execute() << endl;
                else
                    break;
            }
            cout << endl
                 << "Total instructions executed = " << instr_cnt << endl
                 << endl;
            break;
        case 'k':
            while(true)
            {
                if (status_of_program() == 1)
                    cout << execute() << endl;
                else
                    break;
            }
            memory_dump(cout);
            break;

        case 'n': // print specified number of instructions
        {
            int a = 0;
            cin >> a;
            int i = 0;

            for (int i = 0; i < a; i++)
            {
                if (status_of_program() == 1)
                    cout << execute() << endl;
            }
            cout << endl;
        }
        break;
        case 'q': // show current architectural state
            cout << current_state() << endl
                 << endl;
            break;
        case 'u': // disassemble
            disassemble(cout);
            cout << endl;
            break;
        case 'd': // outputs memory dump
            memory_dump(cout);
            break;
        case 'c': // number of instructions executed thus far
            cout << "Number of instructions executed = " << instr_cnt << endl
                 << endl;
            break;
        case 'x': // QUIT
            cout << "Emulation terminated" << endl
                 << endl;
            break;
        default:
            cout << "Please choose from given options" << endl;
            string options = print_options();
            cout << options;
            cout << endl;
            continue;
            break;
        }
    } while (opt != 'x');
    cout << nounitbuf;
    return 0; // Exiting code with success
}
