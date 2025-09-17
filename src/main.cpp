#include <iostream>
#include "cpu.hpp"


using namespace std;



int main() {
    CPU meuSistema;
    
    meuSistema.boot();
    meuSistema.executar();

    return 0;
}