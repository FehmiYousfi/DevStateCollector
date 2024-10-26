#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

std::string exec_command(const char* command) {
    char buffer[128];
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }

    return result;
}

int main() {
    // const char* command = "ls -l"; 
    // try {
    //     std::string output = exec_command(command);
    //     std::cout << "Command output:\n" << output << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    // }

    return 0;
}
