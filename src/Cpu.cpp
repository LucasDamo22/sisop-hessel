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

void CPU::escalonador() {
    std::cout << "\n=======================   ESCALONADOR ACIONADO    =======================" << std::endl;
    //mantem prioridade para RR-real time
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

    //verifica se há algum processo em wait, adiciona na fila se deu timeout do wait
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
    std::cout<< "\n===========================   EXECUTANDO OS   ===========================" << std::endl;
    
    int next_sched_time = 0;
    while(1){
        if((elapsed_time == next_sched_time) || this->running == nullptr || (this->running->sched == Scheduling::FCFS)){
            // std::cout<< "escalonador tempo " << elapsed_time << std::endl;
            escalonador();
            next_sched_time = this->QUANTUM + elapsed_time;
            
        }
        // std::cout<< "next sched time "<< next_sched_time << std::endl;
        std::cout<<"\n--- Elapsed Time: " << elapsed_time <<std::endl;

        if(running != nullptr){
            std::cout << "--- Processo em execucao: " << this->running->id <<std::endl;
            std::cout << "--- Sched: " << scheduling_to_string(this->running->sched) <<std::endl;
        } else {
            std::cout<<"--- CPU em idle "<<std::endl;
        }
        executarInstrucao();
        elapsed_time++;
        std::this_thread::sleep_for(std::chrono::milliseconds(SIM_STEP));
    }
}

// Implementação da execução de uma instrução
bool CPU::executarInstrucao() {

    if (this->running == nullptr) {
        return false;
    }

    Instrucao instr = this->running->codigo[this->running->pc];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(3, 5);

    switch (instr.opcode) {
        // ===== SYSCALL =====
        case OpCode::SYSCALL:
            switch (std::stoi(instr.operando_str)) {
            case 0: // EXIT
                std::cout << "\n[SYSCALL 0 - EXIT] Processo:" << running->id << std::endl;
                std::cout << "  Encerrando processo." << std::endl;
                this->exit.push_back(this->running);
                this->running = nullptr;
                break;

            case 1: // PRINT
                std::cout << "\n[SYSCALL 1 - PRINT] Processo:" << running->id << std::endl;
                std::cout << "  Saída (ACC) = " << this->running->acc << std::endl;
                this->running->wait_time = elapsed_time + distrib(gen);
                waiting.push_back(this->running);
                this->running->pc++;
                this->running = nullptr;
                break;

            case 2: // READ
                std::cout << "\n[SYSCALL 2 - READ] Processo:" << running->id << std::endl;
                std::cout << "  Digite um valor: ";
                std::cin >> this->running->acc;
                this->running->wait_time = elapsed_time + distrib(gen);//gera um valor de wait  de 3/5
                waiting.push_back(this->running);
                this->running->pc++;
                this->running = nullptr;
                break;

            default:
                std::cout << "\n[SYSCALL] Operando inválido" << std::endl;
                break;
            }
            break;

        // ===== ADD =====
        case OpCode::ADD:
            std::cout << "\n[ADD] Processo: " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;

            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Somando imediato = " << instr.operando_val << std::endl;
                running->acc += instr.operando_val;
            } else {
                std::cout << "  Somando memória[" << instr.operando_str << "] = "
                        << running->dados[instr.operando_str] << std::endl;
                running->acc += running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois = " << running->acc << std::endl;
            break;

        // ===== SUB =====
        case OpCode::SUB:
            std::cout << "\n[SUB] Processo: " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Subtraindo imediato = " << instr.operando_val << std::endl;
                running->acc -= instr.operando_val;
            } else {
                std::cout << "  Subtraindo memória[" << instr.operando_str << "]="
                        << running->dados[instr.operando_str] << std::endl;
                running->acc -= running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois = " << running->acc << std::endl;
            break;

        // ===== MULT =====
        case OpCode::MULT:
            std::cout << "\n[MULT] Processo: " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;
            if(instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Multiplicando por imediato =" << instr.operando_val << std::endl;
                running->acc *= instr.operando_val;
            } else {
                std::cout << "  Multiplicando por memória[" << instr.operando_str << "] = "
                        << running->dados[instr.operando_str] << std::endl;
                running->acc *= running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois = " << running->acc << std::endl;
            break;

        // ===== DIV =====
        case OpCode::DIV:
            std::cout << "\n[DIV] Processo " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;
            {
                int divisor = (instr.modo == ModoEnderecamento::IMEDIATO)
                    ? instr.operando_val
                    : running->dados[instr.operando_str];

                std::cout << "  Divisor =" << divisor << std::endl;

                if (divisor != 0) {
                    running->acc /= divisor;
                    std::cout << "  ACC depois = " << running->acc << std::endl;
                } else {
                    std::cout << "  ERRO: divisão por zero!" << std::endl;
                }
            }
            break;

        // ===== LOAD =====
        case OpCode::LOAD:
            std::cout << "\n[LOAD] Processo: " << running->id 
                    << "    PC = " << running->pc;
            if (instr.modo == ModoEnderecamento::IMEDIATO) {
                std::cout << "  Carregando imediato " << instr.operando_val << " em ACC" << std::endl;
                running->acc = instr.operando_val;
            } else {
                std::cout << "  Carregando memória[" << instr.operando_str << "] = "
                        << running->dados[instr.operando_str] << " em ACC" << std::endl;
                running->acc = running->dados[instr.operando_str];
            }
            std::cout << "  ACC depois = " << running->acc << std::endl;
            break;

        // ===== STORE =====
        case OpCode::STORE:
            std::cout << "\n[STORE] Processo: " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;
            if (instr.modo == ModoEnderecamento::DIRETO) {
                running->dados[instr.operando_str] = running->acc;
                std::cout << "  Memória[" << instr.operando_str << "] = "
                        << running->acc << std::endl;
            } else {
                std::cout << "  ERRO: STORE só aceita endereçamento DIRETO" << std::endl;
            }
            break;

        // ===== BRANY =====
        case OpCode::BRANY:
            std::cout << "\n[BRANY] Processo: " << running->id 
                    << "    PC = " << running->pc;
            running->pc = instr.operando_val - 1;
            std::cout << "  Salto incondicional para PC = " << instr.operando_val << std::endl;
            break;

        // ===== BRPOS =====
        case OpCode::BRPOS:
            std::cout << "\n[BRPOS] Processo: " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;
            if (running->acc > 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "  ACC > 0, salto para PC = " << instr.operando_val << std::endl;
            } else {
                std::cout << "  ACC <= 0, não saltou" << std::endl;
            }
            break;

        // ===== BRZERO =====
        case OpCode::BRZERO:
            std::cout << "\n[BRZERO] Processo " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;
            if (running->acc == 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "  ACC == 0, salto para PC = " << instr.operando_val << std::endl;
            } else {
                std::cout << "  ACC != 0, não saltou" << std::endl;
            }
            break;

        // ===== BRNEG =====
        case OpCode::BRNEG:
            std::cout << "\n[BRNEG] Processo " << running->id 
                    << "    PC = " << running->pc
                    << "    ACC antes = " << running->acc << std::endl;
            if (running->acc < 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "  ACC < 0, salto para PC=" << instr.operando_val << std::endl;
            } else {
                std::cout << "  ACC >= 0, não saltou" << std::endl;
            }
            break;

        // ===== INVALIDO =====
        case OpCode::INVALIDO:
        default:
            std::cout << "\n[ERRO] Processo " << running->id 
                    << " | PC=" << running->pc 
                    << ", instrução inválida!" << std::endl;
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