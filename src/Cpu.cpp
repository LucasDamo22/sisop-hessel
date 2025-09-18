#include "cpu.hpp"       
#include "parser.hpp"   
#include <iostream>     
#include <chrono> 
#include <thread> 
#include <algorithm>
#include <random>
#include <sstream>
#include <cstdlib>

#define SIM_STEP 1000

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

CPU::CPU() : running(nullptr), pid(0), elapsed_time(0) {}

void CPU::boot() {
    Parser meuParser;
    
    std::cout << "\n==============================  BOOT DO OS  ==============================" << std::endl;

    std::cout << "Criando Processo 1..." << std::endl;
    processos.push_back(meuParser.parse("programs/prog1.txt"));
    std::cout << "Criando Processo 2..." << std::endl;
    processos.push_back(meuParser.parse("programs/prog2.txt"));
    std::cout << "Criando Processo 3..." << std::endl;
    processos.push_back(meuParser.parse("programs/prog3.txt"));
    std::cout << "Criando Processo 4..." << std::endl;
    processos.push_back(meuParser.parse("programs/prog4.txt"));
    std::cout << "Criando Processo 5..." << std::endl;
    processos.push_back(meuParser.parse("programs/prog5.txt"));
    std::cout << "Criando Processo 6..." << std::endl;
    processos.push_back(meuParser.parse("programs/prog6.txt"));
    
    std::cout << "\n==========================  PROCESSOS CRIADOS:  =========================" << std::endl;
    for(size_t i = 0; i < processos.size(); i++ ){
        this->newprocess.push_back(&processos[i]);
        this->newprocess[i]->id = i;
        newprocess.back()->tostr();
    }
}

void CPU::insere_realtime(Processo *p1){
    auto comparador = [](const Processo* p1, const Processo* p2) {
        return p1->prio < p2->prio;
    };
    auto it = std::upper_bound(this->real_time.begin(), this->real_time.end(), p1, comparador);
    this->real_time.insert(it, p1);
}

void CPU::escalonador(std::stringstream& logger) {
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

    if(!waiting.empty()) {
        for(auto it = waiting.begin(); it != waiting.end();) {
            Processo *current = *it;
            if(this->elapsed_time >= current->wait_time){
                if(current->sched == Scheduling::RR){
                    insere_realtime(current);
                } else {
                    this->best_effort.push_back(current);
                }
                it = waiting.erase(it);
            }else{
                it++;
            }
        }
    }

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

void CPU::executar() {
    std::cout<< "\n===========================   EXECUTANDO OS   ===========================" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    int next_sched_time = 0;
    while(1){
        std::stringstream logger;
        
        if((elapsed_time >= next_sched_time) || this->running == nullptr){
            logger << "=======================   ESCALONADOR ACIONADO    =======================" << std::endl;
            escalonador(logger);
            next_sched_time = this->QUANTUM + elapsed_time;
        } else {
            logger << "=======================   ESCALONADOR   IDLE      =======================" << std::endl;
        }
        
        
        logger << "=========================== FILAS DE PROCESSOS ===========================" << std::endl;
        logger << "Novos      (" << newprocess.size() << "): ";
        for (const auto* p : newprocess) { logger << "P" << p->id << " "; }
        logger << std::endl;
        logger << "Real-Time  (" << real_time.size() << "): ";
        for (const auto* p : real_time) { logger << "P" << p->id << "(p" << p->prio << ") "; }
        logger << std::endl;
        logger << "Best-Effort(" << best_effort.size() << "): ";
        for (const auto* p : best_effort) { logger << "P" << p->id << " "; }
        logger << std::endl;
        logger << "Aguardando (" << waiting.size() << "): ";
        for (const auto* p : waiting) { logger << "P" << p->id << " "; }
        logger << std::endl;

        

        logger <<"\n--- Elapsed Time: " << elapsed_time <<std::endl;
        logger << "--- Proximo Escalonamento: " << next_sched_time << std::endl;

        if(running != nullptr){
            logger << "--- Processo em execucao: " << this->running->id <<std::endl;
            logger << "--- Sched: " << scheduling_to_string(this->running->sched) <<std::endl;
            logger << "--- ACC: " << this->running->acc << std::endl;
            if (running->pc < running->codigo.size()) {
                const auto& instr = running->codigo[running->pc];
                logger << "--- Prox. Instrucao (PC=" << running->pc << "): "
                       << opcode_to_string(instr.opcode) << " " << instr.operando_str << std::endl;
            } else {
                logger << "--- Prox. Instrucao: Nenhuma (fim do programa)" << std::endl;
            }
        } else {
            logger <<"--- CPU em idle "<<std::endl;
        }
        
        executarInstrucao(logger);
        
        clear_screen();
        std::cout << logger.str() << std::flush;
        
        elapsed_time++;
        std::this_thread::sleep_for(std::chrono::milliseconds(SIM_STEP));
    }
}

bool CPU::executarInstrucao(std::stringstream& logger) {
    if (this->running == nullptr) {
        return false;
    }

    if (this->running->pc >= this->running->codigo.size()) {
        this->exit.push_back(this->running);
        this->running = nullptr;
        return false;
    }

    Instrucao instr = this->running->codigo[this->running->pc];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(3, 5);

    switch (instr.opcode) {
        case OpCode::SYSCALL:
            switch (std::stoi(instr.operando_str)) {
            case 0:
                this->exit.push_back(this->running);
                this->running = nullptr;
                break;
            case 1:
                logger << "[SYSCALL] Processo " << running->id << " imprimiu: " << this->running->acc << std::endl;
                this->running->wait_time = elapsed_time + distrib(gen);
                waiting.push_back(this->running);
                this->running->pc++;
                this->running = nullptr;
                break;
            case 2:
                logger << "\n[SYSCALL 2 - READ] Processo " << running->id << " aguardando entrada..." << std::endl;
                clear_screen();
                std::cout << logger.str() << std::flush;
                std::cin >> this->running->acc;
                this->running->wait_time = elapsed_time + distrib(gen);
                waiting.push_back(this->running);
                this->running->pc++;
                this->running = nullptr;
                break;
            default:
                logger << "\n[SYSCALL] Operando invalido para o Processo " << running->id << std::endl;
                break;
            }
            break;
        case OpCode::ADD:
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                running->acc += instr.operando_val;
            } else {
                running->acc += running->dados[instr.operando_str];
            }
            break;
        case OpCode::SUB:
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                running->acc -= instr.operando_val;
            } else {
                running->acc -= running->dados[instr.operando_str];
            }
            break;
        case OpCode::MULT:
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                running->acc *= instr.operando_val;
            } else {
                running->acc *= running->dados[instr.operando_str];
            }
            break;
        case OpCode::DIV:
            {
                int divisor = (instr.modo == ModoEnderecamento::IMEDIATO)
                    ? instr.operando_val
                    : running->dados[instr.operando_str];
                if (divisor != 0) {
                    running->acc /= divisor;
                } else {
                    logger << "  ERRO: divisao por zero no Processo " << running->id << "!" << std::endl;
                }
            }
            break;
        case OpCode::LOAD:
            if (instr.modo == ModoEnderecamento::IMEDIATO) {
                running->acc = instr.operando_val;
            } else {
                running->acc = running->dados[instr.operando_str];
            }
            break;
        case OpCode::STORE:
            if (instr.modo == ModoEnderecamento::DIRETO) {
                running->dados[instr.operando_str] = running->acc;
            } else {
                logger << "  ERRO: STORE so aceita enderecamento DIRETO no Processo " << running->id << std::endl;
            }
            break;
        case OpCode::BRANY:
            running->pc = instr.operando_val - 1;
            break;
        case OpCode::BRPOS:
            if (running->acc > 0) {
                running->pc = instr.operando_val - 1;
            }
            break;
        case OpCode::BRZERO:
            if (running->acc == 0) {
                running->pc = instr.operando_val - 1;
            }
            break;
        case OpCode::BRNEG:
            if (running->acc < 0) {
                running->pc = instr.operando_val - 1;
            }
            break;
        case OpCode::INVALIDO:
        default:
            logger << "\n[ERRO] Processo " << running->id << " | PC=" << running->pc << ", instrucao invalida!" << std::endl;
            break;
    }

    if (this->running != nullptr) {
        this->running->pc++;
    }
    return true;
}

void CPU::print_all_process(){
    for(size_t i = 0; i < processos.size(); i++){
        processos[i].tostr();
    }
}