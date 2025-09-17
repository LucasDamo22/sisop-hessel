#include "cpu.hpp"       
#include "parser.hpp"   
#include <iostream>     
#include <chrono> 
#include <thread> 
#include <algorithm>
#define SIM_STEP 500


CPU::CPU() : running(nullptr), pid(0), elapsed_time(0) {}

void CPU::boot() {
    Parser meuParser;
    processos.push_back(meuParser.parse("programs/prog1.txt"));
    processos.push_back(meuParser.parse("programs/prog2.txt"));
    processos.push_back(meuParser.parse("programs/prog3.txt"));
    for(size_t i = 0; i < processos.size(); i++ ){
        this->newprocess.push_back(&processos[i]);
        this->newprocess[i]->id = i;
        // newprocess.back()->tostr();
    }
}

void CPU::insere_realtime(Processo *p1){
    auto comparador = [](const Processo* p1, const Processo* p2) {
        return p1->prio < p2->prio;
    };

    auto it = std::lower_bound(this->real_time.begin(), this->real_time.end(), p1, comparador);

    this->real_time.insert(it, p1);

}

void CPU::escalonador() {
    if(!newprocess.empty()) {
        for(auto it = newprocess.begin(); it != newprocess.end();) {
            Processo *current = *it;
            
            if(this->elapsed_time >= current->arrival_time){
                if(current->sched == Scheduling::RR){
                    insere_realtime(current);
                } else {
                    this->best_effort.push_back(current);
                }
                it = newprocess.erase(it);
            }else{
                it++;
            }
        }
    }
}


// Implementação do loop principal de execução
void CPU::executar() {
    while(1){
        escalonador();
        elapsed_time++;
        std::cout<<"=========" << std::endl;
        std::cout<<elapsed_time<<std::endl;
        if(!this->real_time.empty()){
            for(size_t i = 0; i < this->real_time.size(); i++){
                std::cout<<"RR ID "<<this->real_time[i]->id<<std::endl;
            }
        }
        if(!this->best_effort.empty()){
            for(size_t i = 0; i < this->best_effort.size(); i++){
                std::cout<<"fsfc ID "<<this->best_effort[i]->id<<std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SIM_STEP));
    }
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