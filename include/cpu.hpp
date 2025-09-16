#include "processo.hpp"
#include "parser.hpp"
#include <queue>
#include <vector>
#include <iostream> // Adicionado para feedback

class CPU {
public:
    int processos_ativos;
    const int QUANTUM = 4; // Quantum para processos best_effort

    // Filas agora guardam ponteiros para os processos
    std::queue<Processo*> real_time;
    std::queue<Processo*> best_effort;
    
    // O vector será o "dono" dos objetos Processo
    std::vector<Processo> processos; 
    
    Processo* processoExecutando;

    CPU() : processos_ativos(0), processoExecutando(nullptr) {}

    bool executarInstrucao(Processo* p); // Função auxiliar

    void boot();
    void executar();
};

void CPU::boot() {
    Parser meuParser;

    // Carrega os processos e os armazena no vector
    processos.push_back(meuParser.parse("data/prog1.txt"));
    processos.push_back(meuParser.parse("data/prog2.txt"));
    processos.push_back(meuParser.parse("data/prog3.txt"));
    processos.push_back(meuParser.parse("data/prog4.txt"));
    processos.push_back(meuParser.parse("data/prog5.txt"));
    processos.push_back(meuParser.parse("data/prog6.txt"));
    
    processos_ativos = processos.size();

    // Distribui os processos nas filas de prontos corretas
    for (int i = 0; i < processos.size(); i++) {
        processos[i].id = i + 1; // Atribui um ID
        processos[i].estado = Estado::READY;

        // Supondo que o parser define a prioridade/scheduling
        if (processos[i].sched == Scheduling::FCFS) { // FCFS tratado como Real-Time
            real_time.push(&processos[i]);
        } else { // RR tratado como Best-Effort
            best_effort.push(&processos[i]);
        }
    }
}

void CPU::executar() {
    while (processos_ativos > 0) {
        
        // 1. ESCOLHA DO PROCESSO (Escalonador com Prioridade)
        if (!real_time.empty()) {
            processoExecutando = real_time.front();
            real_time.pop();
        } else if (!best_effort.empty()) {
            processoExecutando = best_effort.front();
            best_effort.pop();
        } else {
            // CPU Ociosa
            cout << "CPU OCIOSA" << endl;
            continue; 
        }

        processoExecutando->estado = Estado::RUNNING;
        
        int ciclos = (processoExecutando->sched == Scheduling::FCFS) ? 9999 : QUANTUM;//manter o tempo dele???
        bool terminou = false;

        // 2. EXECUÇÃO DO PROCESSO
        for (int i = 0; i < ciclos; i++) {
            if (!executarInstrucao(processoExecutando)) {
                terminou = true;
                break;
            }
        }
        
        // 3. GERENCIAMENTO PÓS-EXECUÇÃO
        if (terminou) {
            processoExecutando->estado = Estado::EXIT;
            processos_ativos--;
        } else {
            processoExecutando->estado = Estado::READY;
            if (processoExecutando->sched == Scheduling::FCFS) {
                // Em um FCFS real com I/O, iria para uma fila de espera.
                // Aqui, apenas assumimos que ele não volta para a fila.
            } else {
                best_effort.push(processoExecutando);
            }
        }
        processoExecutando = nullptr;
    }
    std::cout << "Todos os processos terminaram." << std::endl;
}

// Implementação da execução de uma instrução (baseado na lógica anterior)
bool CPU::executarInstrucao(Processo* p) {
    if (p->pc >= p->codigo.size()) {
        p->estado = Estado::EXIT;
        return false;
    }

    Instrucao instr = p->codigo[p->pc];

    switch (instr.opcode) {
        // ... Lógica para cada opcode (ADD, LOAD, SYSCALL, etc.) ...
        case OpCode::ADD:
            break;
        case OpCode::SUB:
            break;
        case OpCode::MULT:
            break;
        case OpCode::DIV:
            break;
        case OpCode::LOAD:
            break;
        case OpCode::STORE:
            break;
        case OpCode::BRANY:
            break;
        case OpCode::BRPOS:
            break;
        case OpCode::BRZERO:
            break;
        case OpCode::BRNEG:
            break;
        case OpCode::SYSCALL:
            break;
        case OpCode::INVALIDO:
            break;
    }

    p->pc++;
    return true;
}