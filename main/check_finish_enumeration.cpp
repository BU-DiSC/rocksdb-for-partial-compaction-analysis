#include <fstream>
#include <iostream>

bool CheckFinishEnumeration(const std::string& path){
    std::string file_path = path + "/history/picking_history_level1";
    std::cout << file_path << std::endl;
    std::ifstream f(file_path);
    if(!f.is_open()){
        std::cout << "fail to open file: " << file_path << std::endl;
        exit(-1);
    }
    // the fully enumerating sign is on the fourth line, fifth value
    size_t line = 4;
    size_t column = 5;
    // skip line
    std::string skip_line;
    // skip the first line
    std::getline(f, skip_line);
    std::cout << skip_line << std::endl;
    // version_nodes
    size_t vn_size;
    f >> vn_size;
    std::cout << vn_size << std::endl;
    if(vn_size == 0){
        return false;
    }
    std::getline(f, skip_line);
    std::getline(f, skip_line);
    for(size_t i=0; i<column-1; i++){
        f >> skip_line;
    }
    std::string fully_enumerated;
    f >> fully_enumerated;
    std::cout << fully_enumerated << std::endl;
    if(fully_enumerated == "yes"){
        return true;
    }
    return false;
}

int main(int argc, char* argv[]){
    if(CheckFinishEnumeration(argv[1])){
        std::cout << "Finish enumerating" << std::endl;
        // create a file to tell run_all script to stop enumerating
        std::string over_filename = argv[1];
        over_filename += "/over";
        std::ofstream f(over_filename);
        f << "over" << std::endl;
        f.close();
    }
    return 0;
}
