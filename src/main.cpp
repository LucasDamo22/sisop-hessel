#include <iostream>
#include "cpu.hpp"


using namespace std;



int main() {
    CPU meuSistema;
    
    meuSistema.boot();
    meuSistema.print_all_process();
    // meuSistema.executar();

    return 0;
}