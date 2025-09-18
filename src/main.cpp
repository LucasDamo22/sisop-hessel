#include <iostream>
#include "cpu.hpp"

int main() {
    CPU meuSistema;
    
    meuSistema.boot();
    meuSistema.executar();

    return 0;
}