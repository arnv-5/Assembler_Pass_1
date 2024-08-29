#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <optional>
#include <iomanip>  // For std::setw and std::setfill
#include <sstream>  // For std::ostringstream

using namespace std;

#include <algorithm>

// Function to compare SYMTAB entries by their IDs
bool compareSymTab(const pair<string, array<int,3>>& a, const pair<string, array<int,3>>& b) {
    return a.second[0] < b.second[0]; // Compare based on symbol ID
}

// Function to compare LITTAB entries by their IDs
bool compareLitTab(const pair<pair<string, int>, array<int,2>>& a, const pair<pair<string, int>, array<int,2>>& b) {
    return a.second[0] < b.second[0]; // Compare based on literal ID
}

bool isValidInt(const std::string& str) {
    try {
        int num = std::stoi(str);
        // If std::stoi succeeds, it means the string is a valid integer
        return true;
    } catch (const std::invalid_argument& e) {
        // If std::stoi throws an invalid_argument exception, the string is not a valid integer
        return false;
    } catch (const std::out_of_range& e) {
        // If std::stoi throws an out_of_range exception, the number is too large to fit in an int
        return false;
    }
}

bool isVariableNameValid(const std::string& varName) {
    // Check if all characters in varName are alphabetic letters
    return std::all_of(varName.begin(), varName.end(), [](char c) {
        return std::isalpha(static_cast<unsigned char>(c));
    });
}

std::pair<bool, std::pair<std::string, int>> parseVariableExpression(const std::string& operand) {
    std::regex regexPattern(R"(([a-zA-Z]+)([+-]\d+))");
    std::smatch match;

    if (std::regex_match(operand, match, regexPattern)) {
        std::string variable = match[1].str();
        int number = std::stoi(match[2].str());
        return {true, {variable, number}};
    }

    return {false, {"", 0}};  // Return false if the pattern doesn't match
}

const std::pair<const std::pair<std::string, int>, std::array<int, 2>>*
findLiteralById(const std::map<std::pair<std::string, int>, std::array<int, 2>>& LITTAB, int Lit_id) {
    for (const auto& entry : LITTAB) {
        if (entry.second[0] == Lit_id) {
            return &entry;
        }
    }
    return nullptr;  // Return nullptr if the Lit_id is not found
}

int extractNumberFromLiteral(const std::string& literal) {
    std::string numberStr;

    // Start after the '=' character and continue until the end of the string
    for (size_t i = 1; i < literal.size(); ++i) {
        if (std::isdigit(literal[i])) {
            numberStr += literal[i];
        }
    }

    // Convert the extracted string to an integer
    return std::stoi(numberStr);
}

std::string formatNumberWithLeadingZeros(int number) {
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << number;
    return oss.str();
}

// Trim leading and trailing whitespace from a string
std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return ""; // No non-whitespace characters

    const auto end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

void printICTable(const std::vector<std::string>& intermediateCode) {
    // Define column widths
    const int locWidth = 17;
    const int labelWidth = 7;
    const int opcodeWidth = 17;
    const int operand1Width = 10;
    const int operand2Width = 10;

    // Print header
    std::cout << std::left  // Align text to the left
              << std::setw(locWidth) << "Location_Counter"
              << std::setw(labelWidth) << "Label"
              << std::setw(opcodeWidth) << "Mnemonic_Opcode"
              << std::setw(operand1Width) << "Operand1"
              << std::setw(operand2Width) << "Operand2" << std::endl;

    // Print separator line
    std::cout << std::string(locWidth, '-') << ' '
              << std::string(labelWidth, '-') << ' '
              << std::string(opcodeWidth, '-') << ' '
              << std::string(operand1Width, '-') << ' '
              << std::string(operand2Width, '-') << std::endl;

    // Print each entry in the intermediate code table
    for (const auto& entry : intermediateCode) {
        std::istringstream iss(entry);
        std::string loc, label, mnemonic, op1, op2;

        std::getline(iss, loc, ',');
        std::getline(iss, label, ',');
        std::getline(iss, mnemonic, ',');
        std::getline(iss, op1, ',');
        std::getline(iss, op2);

        // Trim leading and trailing spaces from each part
        loc = trim(loc);
        label = trim(label);
        mnemonic = trim(mnemonic);
        op1 = trim(op1);
        op2 = trim(op2);

        // Print each entry in a formatted manner
        std::cout << std::left
                  << std::setw(locWidth) << loc
                  << std::setw(labelWidth) << label
                  << std::setw(opcodeWidth) << mnemonic
                  << std::setw(operand1Width) << op1
                  << std::setw(operand2Width) << op2 << std::endl;
    }
}

int main() {
    std::ifstream inputFile;
    std::ofstream outputFile;
    
    try {
        inputFile.open("input.txt");
        if (!inputFile.is_open()) {
            throw std::runtime_error("Unable to open input file");
        }

        outputFile.open("IC.txt");
        if (!outputFile.is_open()) {
            throw std::runtime_error("Unable to open output file");
        }

        std::string line;
        std::vector<std::string> tokens(4, "-"); // Store exactly 4 tokens, defaulting to "-"
        
        int location_counter = 0;
        vector <string> intermediateCode;
        map <string, array<int,3>> SYMTAB; //SYMTAB["symbol_name"] = {symbol_id, symbol_address, symbol_length};
        map <pair<string,int>, array<int,2>> LITTAB; //LITTAB[{"literal_name",literal_pool_number}] = {literal_id, literal_address};
        int num_literal_pool = 1;
        vector <int> POOLTAB;
        int literal_counter = 0;
        int symbol_counter = 0;
        int rem_lit = 0;
        int to_serve_lit = 0;
        int occurence = 0;
        int ltorg_counter = 0;
        
        map <string,string> IS;
        map <string,string> DL;
        map <string,string> AD;
        map <string,string> Registers;
        map <string,string> BC_Tab;
        
        IS["STOP"] = "00";
        IS["ADD"] = "01";
        IS["SUB"] = "02";
        IS["MULT"] = "03";
        IS["MOVER"] = "04";
        IS["MOVEM"] = "05";
        IS["COMP"] = "06";
        IS["BC"] = "07";
        IS["DIV"] = "08";
        IS["READ"] = "09";
        IS["PRINT"] = "10";
        
        DL.insert(make_pair("DC", "01"));
        DL.insert(make_pair("DS", "02"));
        
        AD.emplace("START","01");
        AD.emplace("END","02");
        AD.emplace("ORIGIN","03");
        AD.emplace("EQU","04");
        AD.emplace("LTORG","05");
        
        Registers["AREG"]="01";
        Registers["BREG"]="02";
        Registers["CREG"]="03";
        Registers["DREG"]="04";
        
        BC_Tab["LT"] = "01";
        BC_Tab["LE"] = "02";
        BC_Tab["EQ"] = "03";
        BC_Tab["GT"] = "04";
        BC_Tab["GE"] = "05";
        BC_Tab["ANY"] = "06";

        while (std::getline(inputFile, line)) {
            std::stringstream ss(line);
            std::string token;

            int tokenIndex = 0;
            while (std::getline(ss, token, ',')) {
                tokens[tokenIndex++] = token;
            }

            // MAIN PROCESSING for the TOKENS 
            
            // final entries
            string loc = "-";
            string label = "-";
            string mnemonic_opcode = "-";
            string operand1 = "-";
            string operand2 = "-";
            
            //tokens being given names 
            string lab = tokens[0];
            string m_op = tokens[1];
            string op1 = tokens[2];
            string op2 = tokens[3];
            
            if(m_op == "START"){
                location_counter = stoi(op2);
                mnemonic_opcode = "(AD 01)";
                operand2 = "(C "+op2+")";
                intermediateCode.push_back(loc + ", " + label + ", " + mnemonic_opcode + ", " + operand1 + ", " + operand2);
                continue;
            }
            
            // determining the opcode 
            if(AD.find(m_op) != AD.end()){
                mnemonic_opcode = "(AD " + AD[m_op] + ")";
            }
            else if(IS.find(m_op) != IS.end()){
                mnemonic_opcode = "(IS " + IS[m_op] + ")";
            }
            else if(DL.find(m_op) != DL.end()){
                mnemonic_opcode = "(DL " + DL[m_op] + ")";
            }
            
            // handling Registers
            if(Registers.find(op1) != Registers.end()){
                operand1 = Registers[op1];
            }
            
            // handling BC statements
            if(mnemonic_opcode == "(IS 07)"){
                operand1 = BC_Tab[op1];
            }
            
            // finding the number of literal pools
            if(mnemonic_opcode == "(AD 02)" || mnemonic_opcode == "(AD 05)"){
                rem_lit = 0; // at END & LTORG, we will set the undealt literals as 0
                num_literal_pool++;
            }
            
            // no location counter value to print for AD instructions
            if (mnemonic_opcode != "(AD 01)" && mnemonic_opcode != "(AD 02)" && mnemonic_opcode != "(AD 03)" && mnemonic_opcode != "(AD 04)" && mnemonic_opcode != "(AD 05)") {
                loc = to_string(location_counter);
            }
            else{ // AD Statement. therefore reduce location_counter by 1 as otherwise it'd increase it by itself after writing into intermediateCode
                loc = "-";
                location_counter--;
            }
            
            // handling labels. Labels need to be processed first as they lead to entries in SYMTAB 
            if (lab != "-") {
                if (SYMTAB.find(lab) == SYMTAB.end()) {
                    symbol_counter++;
                    SYMTAB[lab][0] = symbol_counter;
                    SYMTAB[lab][1] = location_counter;
                    SYMTAB[lab][2] = 1; // Default length 1 for non-DS/DC
                    label = "(S " + to_string(SYMTAB[lab][0]) + ")";
                } else {
                    SYMTAB[lab][1] = location_counter; // Update address if symbol is redefined
                    SYMTAB[lab][2] = 1; // Update length
                    label = "(S " + to_string(SYMTAB[lab][0]) + ")";
                }
            }
            
            //handling operand2
            //check if it is a literal (starts with a '=' sign)
            pair<string, int> litKey = {op2, num_literal_pool};
            if(op2[0] == '='){
                if(LITTAB.find(litKey) == LITTAB.end()){ //literal does not already exist in LITTAB
                    literal_counter++;
                    LITTAB[litKey][0] = literal_counter;
                    operand2 = "(L "+ to_string(literal_counter) + ")";
                    occurence = 1;
                }
                else{
                    // literal already exists in LITTAB
                    operand2 = "(L "+ to_string(LITTAB[litKey][0]) + ")"; //simply print
                }
                
                if(rem_lit == 0){ 
                    // if rem_lit == 0, it means all the above literals were dealt with. So only the new literal pool starts at this entry
                    POOLTAB.push_back(literal_counter);
                }
                rem_lit++;
            }
            // check if operand2 is a constant
            else if(isValidInt(op2)){
                operand2 = "(C "+op2+")";
            }
            // check if operand2 is a variable (just a string variable and not of kind LOOP+50, BACK-13, etc; i.e., used with ORIGIN or EQU)
            else if(isVariableNameValid(op2)){
                /*if (SYMTAB.find(op2) == SYMTAB.end()) {
                    //does not already exist in the table
                    symbol_counter++;
                    SYMTAB[op2][0] = symbol_counter;
                    SYMTAB[op2][1] = -1; // address
                    SYMTAB[op2][2] = -1; // length
                    operand2 = "(S "+to_string(symbol_counter)+")";
                }*/
                if (SYMTAB.find(op2) == SYMTAB.end()) {
                    symbol_counter++;
                    SYMTAB[op2] = {symbol_counter, -1, -1}; // Initialize with default values
                    operand2 = "(S " + to_string(symbol_counter) + ")";
                }

                else{
                    //symbol already exists in the table
                    operand2 = "(S "+to_string(SYMTAB[op2][0])+")";
                }
            }
            
            // handling location_counter, operand2 and literal for ORIGIN and EQU conditions, that also take care of LOOP+2, etc kinda cases
            if(mnemonic_opcode == "(AD 03)"){
                //for ORIGIN - no literal
                // check if operand2 is purely a variable 
                if(isVariableNameValid(op2)){
                    operand2 = "(S "+to_string(SYMTAB[op2][0])+")";
                    // reset the location counter 
                    location_counter = SYMTAB[op2][1]-1; // -1 since location counter gets incremented at the end automatically
                }
                else if(parseVariableExpression(op2).first){
                    // for LOOP+2, BACK-50 etc kind of statements
                    auto resulttt = parseVariableExpression(op2).second;
                    // variable name = resulttt->first and number = resulttt->second
                    string numb = (resulttt.second > 0) ? "+"+to_string(resulttt.second) : to_string(resulttt.second);
                    operand2 = "(S "+to_string(SYMTAB[resulttt.first][0])+")"+numb;
                    //reset the location counter 
                    location_counter = SYMTAB[resulttt.first][1] + (resulttt.second) -1;
                }
            }
            else if(mnemonic_opcode == "(AD 04)"){
                // there will be literals here 
                label = "(S "+to_string(SYMTAB[lab][0])+")";
                // check if operand2 is purely a variable 
                if(isVariableNameValid(op2)){
                    operand2 = "(S "+to_string(SYMTAB[op2][0])+")";
                    // reset the address of literal 
                    SYMTAB[lab][1] = SYMTAB[op2][1];
                }
                else if(parseVariableExpression(op2).first){
                    // for LOOP+2, BACK-50 etc kind of statements
                    auto resulttt = parseVariableExpression(op2).second;
                    // variable name = resulttt.first and number = resulttt.second
                    string numb = (resulttt.second > 0) ? "+"+to_string(resulttt.second) : to_string(resulttt.second);
                    operand2 = "(S "+to_string(SYMTAB[resulttt.first][0])+")"+numb;
                    //reset the address of literal 
                    SYMTAB[lab][1] = SYMTAB[op2][1] + resulttt.second;
                }
            }
            
            // LTORG and END statement 
            if((mnemonic_opcode == "(AD 05)" || mnemonic_opcode == "(AD 02)") && occurence == 1){
                // To handle literals in the current pool
                int starting_lit_ind = POOLTAB[to_serve_lit];
                rem_lit = literal_counter - starting_lit_ind + 1;  // Set rem_lit based on remaining literals
                
                if(mnemonic_opcode == "(AD 05)" || (mnemonic_opcode == "(AD 02)" && rem_lit>0)){
                    // means there are undealt literals yet 
                    while(rem_lit > 0){
                        location_counter++;
                        int Lit_id_to_find = starting_lit_ind;  // Lit_id you're looking for
                        auto result = findLiteralById(LITTAB, Lit_id_to_find);
                        
                        if (result != nullptr) {
                            // Access the literal name and pool number
                            std::string lit_name = result->first.first;
                            int pool_number = result->first.second;
                        
                            // Access the Lit_id and address
                            int found_Lit_id = result->second[0];
                            int address = result->second[1];
                        
                            // If you need to modify the address, you must do so in a mutable copy
                            std::array<int, 2> mutable_address = result->second;
                            mutable_address[1] = location_counter;  // Modify the copy
                            
                            // Debug output
                            std::cout << "Found Literal: " << lit_name << ", Pool: " << pool_number 
                                      << ", Lit_id: " << found_Lit_id << ", Address: " << mutable_address[1] << std::endl;
                        
                            // Update the map with the new address
                            LITTAB[{lit_name, pool_number}] = mutable_address;
                            int num_1 = extractNumberFromLiteral(lit_name);
                            std::string formatted_operand2 = formatNumberWithLeadingZeros(num_1);
                            
                            // Ensure intermediateCode is pushed correctly
                            intermediateCode.push_back(to_string(location_counter) + ", " + label + ", " + mnemonic_opcode + ", " + operand1 + ", " + formatted_operand2);
                        
                                                    starting_lit_ind++;
                            rem_lit--;
                        } else {
                            // Handle the case where literal is not found
                            std::cerr << "Literal with ID " << Lit_id_to_find << " not found in LITTAB" << std::endl;
                            break;
                        }
                    }
                }
                to_serve_lit++;
            }

            // once operand2 is set to a constant value for a DS or DC, we need to enter that to a symbol Table
            if (mnemonic_opcode == "(DL 01)" || mnemonic_opcode == "(DL 02)") {
                if (SYMTAB.find(lab) == SYMTAB.end()) {
                    symbol_counter++;
                    SYMTAB[lab][0] = symbol_counter;
                    label = "(S "+to_string(symbol_counter)+")";
                }
                SYMTAB[lab][1] = location_counter;
                SYMTAB[lab][2] = (mnemonic_opcode == "(DL 01)") ? 1 : stoi(op2);  // DC is length 1, DS uses op2 length
                
                label = "(S "+to_string(SYMTAB[lab][0]) + ")";
                
                if (mnemonic_opcode == "(DL 02)") {
                    location_counter += SYMTAB[lab][2] - 1; // Update location counter for DS
                }
            }
            
            if(mnemonic_opcode == "(AD 05)"){
                ltorg_counter++;
            }
            
            //just END statement 
            if((mnemonic_opcode == "(AD 02)" && ltorg_counter == 0 && to_serve_lit == 0)){
                intermediateCode.push_back(loc + ", " + label + ", " + mnemonic_opcode + ", " + operand1 + ", " + operand2);
            }
            
            if((mnemonic_opcode != "(AD 02)" || rem_lit != 0) && (mnemonic_opcode != "(AD 05)" || rem_lit != 0)){
                intermediateCode.push_back(loc + ", " + label + ", " + mnemonic_opcode + ", " + operand1 + ", " + operand2);
            }
            
            location_counter++;
            // Reset tokens for the next line, though not strictly necessary since tokens will be overwritten
            tokens.assign(4, "-");
        }
        
        for (const auto& entry : intermediateCode) {
            outputFile << entry << std::endl;
        }
        
        // Print the Symbol Table (SYMTAB) ordered by IDs
        cout << "\nSymbol Table (SYMTAB) ordered by ID:\n";
        cout << "Symbol\tID\tAddress\tLength\n";
        vector<pair<string, array<int,3>>> symtabEntries(SYMTAB.begin(), SYMTAB.end());
        sort(symtabEntries.begin(), symtabEntries.end(), compareSymTab);
        
        for (const auto& sym : symtabEntries) {
            cout << sym.first << "\t" 
                 << sym.second[0] << "\t"
                 << sym.second[1] << "\t"
                 << sym.second[2] << endl;
        }

        // Print the Literal Table (LITTAB) ordered by IDs
        cout << "\nLiteral Table (LITTAB) ordered by ID:\n";
        cout << "Literal\tPool\tID\tAddress\n";
        vector<pair<pair<string, int>, array<int,2>>> littabEntries(LITTAB.begin(), LITTAB.end());
        sort(littabEntries.begin(), littabEntries.end(), compareLitTab);
        
        for (const auto& lit : littabEntries) {
            cout << lit.first.first << "\t" 
                 << lit.first.second << "\t"
                 << lit.second[0] << "\t"
                 << lit.second[1] << endl;
        }
        
        // Print the Pool Table (POOLTAB)
        cout << "\nPool Table (POOLTAB):\n";
        cout << "Pool No.\tLiteral ID\n";
        for (size_t i = 0; i < POOLTAB.size(); ++i) {
            cout << i + 1 << "\t\t" << POOLTAB[i] << endl;
        }
        
        printICTable(intermediateCode);


    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    inputFile.close();
    outputFile.close();

    return 0;
}
