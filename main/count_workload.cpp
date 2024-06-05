#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;

int main(int argc, char* argv[]){
    if(argc != 2){
        std::cout << "There should One parameters: " << std::endl;
        std::cout << "1. The experiment path" << std::endl;
        return -1;
    }

    string experiment_path = argv[1];
    string workload_file_path = experiment_path + "/workload.txt";
    string output_file_path = experiment_path + "/workload_count.txt";

    // opening workload file for the first time
    std::ifstream workload_file;
    workload_file.open(workload_file_path);

    unordered_map<char, int> counters;
    int counter = 0;

    size_t total_bytes = 0;

    while (!workload_file.eof()) {
        char instruction;
        std::string value, key, start_key, end_key;
        workload_file >> instruction;
        counters[instruction]++;
        switch (instruction)
        {
        case 'I': // insert
        case 'U': // update
            workload_file >> key >> value;
            total_bytes += key.length() + value.length();
            break;
        case 'Q': // probe: point query
            workload_file >> key;
            break;

        case 'S': // scan: range query
            workload_file >> start_key >> end_key;
            break;
        case 'D':
            workload_file >> key;
            break;
        default:
            std::cerr << "ERROR: Case match NOT found !!" << std::endl;
            break;
        }
        counter++;
    }
    
    cout << "Total operations: " << counter << endl;
    for(auto& p: counters){
        cout << "Operation " << p.first << ": " << p.second << endl;
    }
    std::ofstream out_file(output_file_path);
    out_file << "total_written_bytes=" << total_bytes << endl; 
    cout << "Total bytes: " << total_bytes << endl;
    return 0;
}