#include "cpu.hpp"       
#include "parser.hpp"   
#include <iostream>     
#include <chrono> 
#include <thread> 
#include <algorithm>
#include <random>
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

    auto it = std::upper_bound(this->real_time.begin(), this->real_time.end(), p1, comparador);

    this->real_time.insert(it, p1);

}

void CPU::escalonador() {
    // admissão de processos
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

    // cpu em idle, pega primeiro de tempo real, se houver
    if(this->running == nullptr) {
        if(!this->real_time.empty()) {
            this->running = real_time.front(); 
            this->real_time.pop_front();
        } else if(!this->best_effort.empty()){
            this->running = best_effort.front();
            this->best_effort.pop_front();
        }
    } else {
        Processo *leaving = this->running;
        // so troca o processo se a fila de tempo real não estiver
        if(!this->real_time.empty()){
            if(leaving->sched == Scheduling::RR){
                insere_realtime(leaving);
            } else {
                best_effort.push_front(leaving);
            }
            this->running = real_time.front(); 
            this->real_time.pop_front();
        }
    }
}


// Implementação do loop principal de execução
void CPU::executar() {
    
    int next_sched_time = 0;
    while(1){
        if((elapsed_time == next_sched_time) || this->running == nullptr || (this->running->sched == Scheduling::FCFS)){
            std::cout<< "escalonador tempo " << elapsed_time << std::endl;
            escalonador();
            next_sched_time = this->QUANTUM + elapsed_time;
            
        }
        std::cout<< "next sched time "<< next_sched_time << std::endl;
        
        std::cout<<"=========" << std::endl;
        std::cout<<elapsed_time<<std::endl;
        if(running != nullptr){
            std::cout<<"running " <<this->running->id<< " sched " << scheduling_to_string(this->running->sched) <<std::endl;
        } else {
            std::cout<<"idle "<<std::endl;
        }
        executarInstrucao();
        elapsed_time++;
        std::this_thread::sleep_for(std::chrono::milliseconds(SIM_STEP));
    }
}

// Implementação da execução de uma instrução
bool CPU::executarInstrucao() {
    // Verifica se o program counter (pc) é válido
    if (this->running == nullptr) {
        return false; // Retorna false para indicar que o processo terminou
    }

    Instrucao instr = this->running->codigo[this->running->pc];
    std::cout << "Executando: Processo " << this->running->id << ", PC=" << this->running->pc << ", OpCode=" << static_cast<int>(instr.opcode) << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(3, 5);

    switch (instr.opcode) {
        case OpCode::SYSCALL:
            switch (instr.operando_val) {
            case 0:
                this->exit.push_back(this->running);
                this->running = nullptr;
                break;
            case 1:
                std::cout << "Id " << this->running << ": " << this->running->acc <<std::endl;
                break;
            case 2:
                std::cin >> this->running->acc;
                this->running->wait_time = elapsed_time + distrib(gen);
                break;
            default:
                break;
            }
            break;
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
        case OpCode::INVALIDO:
            break;
        
        default:
            // Placeholder para outras instruções
            break;
    }
    if(!(this->running == nullptr)){
        this->running->pc++;
    }
     // Avança para a próxima instrução
    return true; // Retorna true para indicar que o processo continua
}