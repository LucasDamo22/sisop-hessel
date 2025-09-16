#include <iostream>
#include "parser.hpp"

void imprimir_processo(const Processo& p) {
    std::cout << "--- Resultado do Parsing ---" << std::endl;
    std::cout << "\n[SEGMENTO DE CODIGO]" << std::endl;
    for (size_t i = 0; i < p.codigo.size(); ++i) {
        const auto& instr = p.codigo[i];
        std::cout << "Endereco " << i << ": "
                  << "OpCode(" << static_cast<int>(instr.opcode) << "), "
                  << "OperandoStr(" << instr.operando_str << "), "
                  << "OperandoVal(" << instr.operando_val << "), "
                  << "Modo(" << (instr.modo == ModoEnderecamento::DIRETO ? "DIRETO" : "IMEDIATO") << ")"
                  << std::endl;
    }

    std::cout << "\n[SEGMENTO DE DADOS]" << std::endl;
    for (const auto& par : p.dados) {
        std::cout << "Variavel '" << par.first << "' = " << par.second << std::endl;
    }
}

int main() {
    Parser meuParser;
    Processo p1 = meuParser.parse("data/prog1.txt");

    imprimir_processo(p1);
    //std::cout << p1.dados["controle"] << std::endl;
    return 0;
}