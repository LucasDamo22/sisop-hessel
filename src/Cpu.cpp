#include "cpu.hpp"      // Inclui a declaração da nossa classe
#include "parser.hpp"   // Necessário para a função boot()
#include <iostream>     // Necessário para as mensagens de status

// Implementação do Construtor


CPU::CPU() : running(nullptr), pid(0) {}

// Implementação do método de inicialização
void CPU::boot() {
    Parser meuParser;
    // processos.push_back(meuParser.parse("programs/prog1.txt"));
    // processos.push_back(meuParser.parse("programs/prog2.txt"));
    processos.push_back(meuParser.parse("programs/prog3.txt"));
    // imprimir_processo(processos[0]);
    for(size_t i = 0; i < processos.size(); i++ ){
        newprocess.push_back(&processos[i]);
        newprocess[i]->id = i;
        newprocess.back()->tostr();
    }
}

void CPU::spawn_proc(){

}

// Implementação do loop principal de execução
void CPU::executar() {
    
}

// Implementação da execução de uma instrução
bool CPU::executarInstrucao(Processo* p) {
    // Verifica se o program counter (pc) é válido
    if (p->pc >= p->codigo.size()) {
        p->estado = Estado::EXIT;
        return false; // Retorna false para indicar que o processo terminou
    }

    Instrucao instr = p->codigo[p->pc];
    std::cout << "Executando: Processo " << p->id << ", PC=" << p->pc << ", OpCode=" << static_cast<int>(instr.opcode) << std::endl;

    switch (instr.opcode) {
        // ... SUA LÓGICA PARA CADA OPCODE (ADD, LOAD, SYSCALL, etc.) DEVE IR AQUI ...
        // Exemplo para SYSCALL 0 (halt)
        case OpCode::SYSCALL:
            if (instr.operando_val == 0) {
                p->estado = Estado::EXIT;
                return false; // Retorna false para indicar que o processo terminou
            }
            // Outros syscalls iriam aqui
            break;
        
        default:
            // Placeholder para outras instruções
            break;
    }

    p->pc++; // Avança para a próxima instrução
    return true; // Retorna true para indicar que o processo continua
}