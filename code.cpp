#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <iomanip>
#include <algorithm>

using namespace std;

// Structure for R-type instructions
struct RType {
    string opcode, funct3, funct7;
};

// R-type instructions
unordered_map<string, RType> rTypeInstructions = {
    {"add",  {"0110011", "000", "0000000"}},
    {"sub",  {"0110011", "000", "0100000"}},
    {"and",  {"0110011", "111", "0000000"}},
    {"or",   {"0110011", "110", "0000000"}},
    {"xor",  {"0110011", "100", "0000000"}},
    {"sll",  {"0110011", "001", "0000000"}},
    {"srl",  {"0110011", "101", "0000000"}},
    {"sra",  {"0110011", "101", "0100000"}}
};

// I-type instructions (opcode + funct3)
unordered_map<string, string> iTypeInstructions = {
    {"addi", "0010011-000"},
    {"andi", "0010011-111"},
    {"ori",  "0010011-110"},
    {"jalr", "1100111-000"},
    {"lw",   "0000011-010"} // Load Word
};

// S-type (store) instructions
unordered_map<string, string> sTypeInstructions = {
    {"sw", "0100011-010"} // Store Word
};

// SB-type (branch) instructions
unordered_map<string, string> sbTypeInstructions = {
    {"beq", "1100011-000"},
    {"bne", "1100011-001"}
};

// U-type instructions
unordered_map<string, string> uTypeInstructions = {
    {"lui",   "0110111"},
    {"auipc", "0010111"}
};

// UJ-type (jump) instructions
unordered_map<string, string> ujTypeInstructions = {
    {"jal", "1101111"}
};

// Register Map
unordered_map<string, int> registerMap = {
    {"x0", 0}, {"x1", 1}, {"x2", 2}, {"x3", 3}, {"x4", 4}, {"x5", 5}, {"x6", 6}, {"x7", 7},
    {"x8", 8}, {"x9", 9}, {"x10", 10}, {"x11", 11}, {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15},
    {"x16", 16}, {"x17", 17}, {"x18", 18}, {"x19", 19}, {"x20", 20}, {"x21", 21}, {"x22", 22}, {"x23", 23},
    {"x24", 24}, {"x25", 25}, {"x26", 26}, {"x27", 27}, {"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31}
};

// Convert integer to binary string of given width
string toBinary(int num, int width) {
    bitset<32> bin(num);
    return bin.to_string().substr(32 - width);
}

// Encode R-type instruction
string encodeRType(string instr, string rd, string rs1, string rs2) {
    RType info = rTypeInstructions[instr];
    return info.funct7 + toBinary(registerMap[rs2], 5) + toBinary(registerMap[rs1], 5) + info.funct3 + toBinary(registerMap[rd], 5) + info.opcode;
}

// Encode I-type instruction
string encodeIType(string instr, string rd, string rs1, int imm) {
    string opcode_funct3 = iTypeInstructions[instr];
    string opcode = opcode_funct3.substr(0, 7);
    string funct3 = opcode_funct3.substr(8);
    return toBinary(imm, 12) + toBinary(registerMap[rs1], 5) + funct3 + toBinary(registerMap[rd], 5) + opcode;
}

// Encode SB-type (branch) instruction
string encodeSBType(string instr, string rs1, string rs2, int offset) {
    string opcode_funct3 = sbTypeInstructions[instr];
    string opcode = opcode_funct3.substr(0, 7);
    string funct3 = opcode_funct3.substr(8);
    string imm = toBinary(offset, 13);
    return imm[0] + imm.substr(2, 6) + toBinary(registerMap[rs2], 5) + toBinary(registerMap[rs1], 5) + funct3 + imm.substr(8, 4) + imm[1] + opcode;
}

// Encode UJ-type (jump) instruction
string encodeUJType(string instr, string rd, int offset) {
    string opcode = ujTypeInstructions[instr];
    string imm = toBinary(offset, 21);
    return imm[0] + imm.substr(10, 10) + imm[9] + imm.substr(1, 8) + toBinary(registerMap[rd], 5) + opcode;
}

int main() {
    ifstream input("input.asm");
    ofstream output("output.mc");

    if (!input.is_open() || !output.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    string line;
    int address = 0x0;
    unordered_map<string, int> labelMap;
    vector<string> lines;

    while (getline(input, line)) {
        stringstream ss(line);
        string first;
        ss >> first;
        if (first.back() == ':') {
            labelMap[first.substr(0, first.size() - 1)] = address;
        } else {
            lines.push_back(line);
            address += 4;
        }
    }

    input.clear();
    input.seekg(0);
    address = 0x0;

    for (const string& line : lines) {
        stringstream ss(line);
        string instr, rd, rs1, rs2;
        int imm;
        string machineCode;

        ss >> instr;
        if (rTypeInstructions.find(instr) != rTypeInstructions.end()) {
            ss >> rd >> rs1 >> rs2;
            rd.pop_back(); rs1.pop_back();
            machineCode = encodeRType(instr, rd, rs1, rs2);
        } else if (iTypeInstructions.find(instr) != iTypeInstructions.end()) {
            ss >> rd >> rs1 >> imm;
            rd.pop_back(); rs1.pop_back();
            machineCode = encodeIType(instr, rd, rs1, imm);
        } else if (sbTypeInstructions.find(instr) != sbTypeInstructions.end()) {
            ss >> rs1 >> rs2 >> imm;
            rs1.pop_back(); rs2.pop_back();
            machineCode = encodeSBType(instr, rs1, rs2, imm);
        } else {
            cerr << "Unsupported instruction: " << instr << endl;
            continue;
        }

        output << "0x" << hex << address << " 0x" << machineCode << " , " << line << endl;
        address += 4;
    }

    cout << "Assembly successfully translated to machine code in output.mc!" << endl;
    return 0;
}
