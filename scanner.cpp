#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <cstdio>

struct memreg {
    uintptr_t start;
    uintptr_t end;
    std::string perms;
};

class memory_scanner {
private:
    int pid;
    std::fstream mem_file;
    std::vector<memreg> m_write_reg;
    std::vector<uintptr_t> target_addresses;
    
public:

    std::string mem_path;
    std::string maps_path;

    memory_scanner(int target_pid) : pid(target_pid) {
        mem_path = "/proc/" + std::to_string(pid) + "/mem";
        maps_path = "/proc/" + std::to_string(pid) + "/maps";
        mem_file.open(mem_path, std::ios::in | std::ios::out | std::ios::binary);
    }

    bool scan_regions() {
        std::ifstream maps_file(maps_path);
        if(!maps_file.is_open()) {
            std::cerr << "Failed to open maps file for PID:" << pid << std::endl;
            return false;
        }

        std::string line;

        while(std::getline(maps_file,line)) {
            uintptr_t start = 0;
            uintptr_t end = 0;
            char perms_buf[5] = {0};

            if(sscanf(line.c_str(), "%lx-%lx %4s", &start, &end, perms_buf) == 3) {
                if(std::string(perms_buf) == "rw-p") {
                    m_write_reg.push_back({start, end, perms_buf});
                }
            }
        }

        return !m_write_reg.empty();
    }

    void print_regions() {
        std::cout << "\nFound Writeable Regions:\n" << std::endl;
        for(const auto& region : m_write_reg) {
            std::cout << std::hex << "0x" << region.start << " - 0x" << region.end 
                        << " [" << region.perms << " ]\n" << std::endl;
        }
    }

    void print_address() {
        for(auto temp : target_addresses) {
            std::cout << temp << std::endl;
        }
    }

    bool mem_write(uintptr_t address, const int& value) {
        mem_file.clear();
        mem_file.seekp(address);
        if(!mem_file) {
            std::cerr << "Failed to seek to address: 0x" << std::hex << address << std::endl;
            return false;
        }

        mem_file.write(reinterpret_cast<const char*> (&value), sizeof(value));
        return mem_file.good();
    }

    bool mem_read(uintptr_t address, int& value) {
        mem_file.clear();

        if(!mem_file.is_open()) {
            std::cerr << "Failed to open mem file. Did you forget 'sudo'?" << std::endl;
            return false;
        }

        mem_file.seekg(address);
        if(!mem_file) {
            std::cerr << "Failed to seek to address: 0x" << std::hex << address << std::endl;
            return false;
        }

        mem_file.read(reinterpret_cast<char*> (&value), sizeof(value));
        return mem_file.good();    
    }

    void filter_val(int value) {
        std::vector<uintptr_t> filter;
        mem_file.clear();

        if(!mem_file.is_open()) {
            std::cerr << "Failed to open file!" << std::endl;
            return;
        }

        for(auto temp : target_addresses) {
            int new_val = 0;

            mem_file.seekg(temp);
            if(mem_file.read(reinterpret_cast<char*> (&new_val), sizeof(new_val))) {
                if(new_val == value) {
                    filter.push_back(temp);
                }    
            }
            mem_file.clear();
        }

        target_addresses = filter;
    }

    void find_value(int target_val) {
        mem_file.clear();
        if(!mem_file.is_open()) {
            std::cerr << "Failed to open mem file. Did you forget 'sudo'?" << std::endl;
            return;
        }

        for(const auto& find : m_write_reg) {
            size_t size = find.end - find.start;
            std::vector<char> buffer(size);
            
            mem_file.clear();
            mem_file.seekg(find.start);
            mem_file.read(buffer.data(), size);

            for(size_t i = 0; i + sizeof(int) <= size; ++i) {
                int val = *reinterpret_cast<int*> (&buffer[i]);
                if(val == target_val) {
                    target_addresses.push_back(find.start + i);
                }
            }
        }

        return;
    }

    bool target_empty() {
        return target_addresses.empty();
    }
    
    uintptr_t get_target_index(size_t index = 0) {
        if(index < target_addresses.size()) {
            return target_addresses[index];
        }
        return 0;
    }

    ~memory_scanner() {
        if(mem_file.is_open()) {
            mem_file.close();
        }
    }
};

int main() {
    int pid = 0;
    std::cout << "Enter target PID" << std::endl;
    std::cin >> pid;

    memory_scanner cheat(pid);
    cheat.scan_regions();
    cheat.print_regions();

    int target_val;
    std::cout << "\nEnter target value to find: ";
    std::cin >> target_val;

    cheat.find_value(target_val);
    if(cheat.target_empty()){
         return -1;
    }

    cheat.print_address();

    while (true)
    {
        int new_val = 0;
        std::cout << "Enter a new value for filtering (or enter -1 to proceed to the stage of changing the value)" << std::endl;
        std::cin >> new_val;

        if(new_val == -1){
            break;
        } else {
            cheat.filter_val(new_val);
            cheat.print_address();
        }
    }
    
    int nval;
    std::cout << "Enter new value for change tho old: " << std::endl;
    std::cin >> nval;

    if(cheat.target_empty()) {
        std::cerr << "Addresses have ended." << std::endl;
        return -1;
    }
    
    uintptr_t target_addr = cheat.get_target_index();

    if(!cheat.mem_write(target_addr, nval)) {
        return -1;
    }

    int read_val = 0;
    cheat.mem_read(target_addr, read_val);

    std::cout << "Target reading from Cheat-Engine: " << std::dec << read_val << std::endl;

    return 0;
}