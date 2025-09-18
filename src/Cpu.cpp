#include "cpu.hpp"       
#include "parser.hpp"   
#include <iostream>     
#include <chrono> 
#include <thread> 
#include <algorithm>
#include <random>
#define SIM_STEP 1000

// using namespace std::cout, std::endl;


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
        this->newprocess[i]->estado = Estado::NEW;
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
                    current->estado = Estado::READY;
                    insere_realtime(current);
                } else {
                    current->estado = Estado::READY;
                    this->best_effort.push_back(current);
                }
                it = newprocess.erase(it);
            }else{
                it++;
            }
        }
    }
    std::cout << "wait queue size "<< waiting.size() << std::endl;
    if(!waiting.empty()) {
        for(auto it = waiting.begin(); it != waiting.end();) {
            Processo *current = *it;
            std::cout << "wait id "<<current->id << std::endl;
            
            if(this->elapsed_time >= current->wait_time){
                if(current->sched == Scheduling::RR){
                    current->estado = Estado::READY;
                    insere_realtime(current);
                } else {
                    current->estado = Estado::READY;
                    this->best_effort.push_back(current);
                }
                it = waiting.erase(it);
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
            this->running->estado = Estado::RUNNING;
            this->running->restante_quantum = this->running->quantum;
        } else if(!this->best_effort.empty()){
            this->running = best_effort.front();
            this->best_effort.pop_front();
            this->running->estado = Estado::RUNNING;
        }
    } else {
        Processo *leaving = this->running;
        // so troca o processo se a fila de tempo real não estiver
        if(!this->real_time.empty()){
            if(leaving->sched == Scheduling::RR){
                leaving->estado = Estado::READY;
                insere_realtime(leaving);
            } else {
                leaving->estado = Estado::READY;
                best_effort.push_front(leaving);
            }
            this->running = real_time.front(); 
            this->real_time.pop_front();
            this->running->estado = Estado::RUNNING;
            this->running->restante_quantum = this->running->quantum;
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
            next_sched_time = (this->running && this->running->sched == Scheduling::RR)
                                ? (elapsed_time + this->running->restante_quantum)
                                : (this->QUANTUM + elapsed_time);
            
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

        if (running && running->sched == Scheduling::RR) {
            if (--running->restante_quantum == 0) {
                Processo* p = running;
                running = nullptr;
                p->estado = Estado::READY;
                insere_realtime(p);
                p->restante_quantum = p->quantum;
                next_sched_time = elapsed_time; 
            }
        }

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

    if (this->running->pc < 0 || this->running->pc >= (int)this->running->codigo.size()) {
        this->running->estado = Estado::EXIT;
        this->exit.push_back(this->running);
        this->running = nullptr;
        return true;
    }

    Instrucao instr = this->running->codigo[this->running->pc];
    std::cout << "Executando: Processo " << this->running->id << ", PC=" << this->running->pc << ", OpCode=" << opcode_to_string(instr.opcode) << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(3, 5);

    switch (instr.opcode) {
        // ===== SYSCALL =====
        case OpCode::SYSCALL:
            switch (instr.operando_val) {
            case 0: // EXIT
                std::cout << "\n[SYSCALL 0 - EXIT] Processo " << running->id
                        << " | PC=" << running->pc << std::endl;
                std::cout << "  Encerrando processo." << std::endl;
                this->running->estado = Estado::EXIT;
                this->exit.push_back(this->running);
                this->running = nullptr;
                break;

            case 1: // PRINT
                std::cout << "\n[SYSCALL 1 - PRINT] Processo " << running->id
                        << " | PC=" << running->pc << std::endl;
                std::cout << "  Saída (ACC)=" << this->running->acc << std::endl;
                this->running->wait_time = elapsed_time + distrib(gen);
                this->running->estado = Estado::WAITING;
                waiting.push_back(this->running);
                this->running->pc++;
                this->running = nullptr;
                break;

            case 2: // READ
                std::cout << "\n[SYSCALL 2 - READ] Processo " << running->id
                        << " | PC=" << running->pc << std::endl;
                std::cout << "  Digite um valor: ";
                std::cin >> this->running->acc;
                if (this->running->acc == 101) std::cout << "Ola" << std::endl;
                this->running->wait_time = elapsed_time + distrib(gen);
                std::cout << "wait time" <<this->running->wait_time <<std::endl;
                this->running->estado = Estado::WAITING;
                waiting.push_back(this->running);
                this->running->pc++;
                this->running = nullptr;
                break;

            default:
                std::cout << "\n[SYSCALL] ERRO: Operando inválido (" 
                        << instr.operando_val << ")" << std::endl;
                break;
            }
            break;

        // ===== ADD =====
        case OpCode::ADD:
            std::cout << "\n[ADD] Processo " << running->id 
                    << " | PC=" << running->pc 
                    << " | ACC antes=" << running->acc << std::endl;
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Somando imediato " << instr.operando_val << std::endl;
                running->acc += instr.operando_val;
            } else {
                std::cout << "  Somando memória[" << instr.operando_str << "]="
                        << running->dados[instr.operando_str] << std::endl;
                running->acc += running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois=" << running->acc << std::endl;
            break;

        // ===== SUB =====
        case OpCode::SUB:
            std::cout << "\n[SUB] Processo " << running->id 
                    << " | PC=" << running->pc 
                    << " | ACC antes=" << running->acc << std::endl;
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Subtraindo imediato " << instr.operando_val << std::endl;
                running->acc -= instr.operando_val;
            } else {
                std::cout << "  Subtraindo memória[" << instr.operando_str << "]="
                        << running->dados[instr.operando_str] << std::endl;
                running->acc -= running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois=" << running->acc << std::endl;
            break;

        // ===== MULT =====
        case OpCode::MULT:
            std::cout << "\n[MULT] Processo " << running->id 
                    << " | PC=" << running->pc 
                    << " | ACC antes=" << running->acc << std::endl;
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Multiplicando por imediato " << instr.operando_val << std::endl;
                running->acc *= instr.operando_val;
            } else {
                std::cout << "  Multiplicando por memória[" << instr.operando_str << "]="
                        << running->dados[instr.operando_str] << std::endl;
                running->acc *= running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois=" << running->acc << std::endl;
            break;

        // ===== DIV =====
        case OpCode::DIV:
            std::cout << "\n[DIV] Processo " << running->id 
                    << " | PC=" << running->pc 
                    << " | ACC antes=" << running->acc << std::endl;
            {
                int divisor = (instr.modo == ModoEnderecamento::IMEDIATO)
                    ? instr.operando_val
                    : running->dados[instr.operando_str];

                std::cout << "  Divisor=" << divisor << std::endl;

                if (divisor != 0) {
                    running->acc /= divisor;
                    std::cout << "  ACC depois=" << running->acc << std::endl;
                } else {
                    std::cout << "  ERRO: divisão por zero!" << std::endl;
                }
            }
            break;

        // ===== LOAD =====
        case OpCode::LOAD:
            std::cout << "\n[LOAD] Processo " << running->id 
                    << " | PC=" << running->pc << std::endl;
            if (instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Carregando imediato " << instr.operando_val << " em ACC" << std::endl;
                running->acc = instr.operando_val;
            } else {
                std::cout << "  Carregando memória[" << instr.operando_str << "]="
                        << running->dados[instr.operando_str] << " em ACC" << std::endl;
                running->acc = running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois=" << running->acc << std::endl;
            break;

        // ===== STORE =====
        case OpCode::STORE:
            std::cout << "\n[STORE] Processo " << running->id 
                    << " | PC=" << running->pc 
                    << " | ACC=" << running->acc << std::endl;
            if (instr.modo == ModoEnderecamento::DIRETO) {
                running->dados[instr.operando_str] = running->acc;
                std::cout << "  Memória[" << instr.operando_str << "]="
                        << running->acc << std::endl;
            } else {
                std::cout << "  ERRO: STORE só aceita endereçamento DIRETO" << std::endl;
            }
            break;

        // ===== BRANY =====
        case OpCode::BRANY:
            std::cout << "\n[BRANY] Processo " << running->id 
                    << " | PC atual=" << running->pc << std::endl;
            running->pc = instr.operando_val - 1;
            std::cout << "  Salto incondicional para PC=" << instr.operando_val << std::endl;
            break;

        // ===== BRPOS =====
        case OpCode::BRPOS:
            std::cout << "\n[BRPOS] Processo " << running->id 
                    << " | PC atual=" << running->pc 
                    << " | ACC=" << running->acc << std::endl;
            if (running->acc > 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "  ACC > 0 → salto para PC=" << instr.operando_val << std::endl;
            } else {
                std::cout << "  ACC <= 0 → não saltou" << std::endl;
            }
            break;

        // ===== BRZERO =====
        case OpCode::BRZERO:
            std::cout << "\n[BRZERO] Processo " << running->id 
                    << " | PC atual=" << running->pc 
                    << " | ACC=" << running->acc << std::endl;
            if (running->acc == 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "  ACC == 0 → salto para PC=" << instr.operando_val << std::endl;
            } else {
                std::cout << "  ACC != 0 → não saltou" << std::endl;
            }
            break;

        // ===== BRNEG =====
        case OpCode::BRNEG:
            std::cout << "\n[BRNEG] Processo " << running->id 
                    << " | PC atual=" << running->pc 
                    << " | ACC=" << running->acc << std::endl;
            if (running->acc < 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "  ACC < 0 → salto para PC=" << instr.operando_val << std::endl;
            } else {
                std::cout << "  ACC >= 0 → não saltou" << std::endl;
            }
            break;

        // ===== INVALIDO =====
        case OpCode::INVALIDO:
        default:
            std::cout << "\n[ERRO] Processo " << running->id 
                    << " | PC=" << running->pc 
                    << " → instrução inválida!" << std::endl;
            break;
    }


    // Incrementa o PC se o processo não terminou ou não saltou
    if (this->running != nullptr) {
        this->running->pc++;
    }
     // Avança para a próxima instrução
    return true; // Retorna true para indicar que o processo continua
}

void CPU::print_all_process(){
    for(int i = 0; i < processos.size(); i++){
        processos[i].tostr();
    }
}
