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
    std::cout << "Executando: Processo " << this->running->id << ", PC=" << this->running->pc << ", OpCode=" << opcode_to_string(instr.opcode) << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(3, 5);

    switch (instr.opcode) {
        // Casos já implementados por você
        case OpCode::SYSCALL:
            switch (instr.operando_val) {
            case 0:
                std::cout << "============EXECUTANDO SYSCALL 0 (EXIT)============" << std::endl;
                this->exit.push_back(this->running);
                this->running = nullptr;
                break;
            case 1:
                std::cout << "============EXECUTANDO SYSCALL 1 (PRINT)===========" << std::endl;
                std::cout << "SAIDA DO PROCESSO ID " << this->running->id << ": " << this->running->acc << std::endl;
                break;
            case 2:
                std::cout << "============EXECUTANDO SYSCALL 2 (READ)============" << std::endl;
                std::cout << "Digite um valor para o processo ID " << this->running->id << ": ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin >> this->running->acc;
                this->running->wait_time = elapsed_time + distrib(gen);
                break;
            default:
                std::cout << "ERRO: SYSCALL com operando invalido." << std::endl;
                break;
            }
            break; // Fim do SYSCALL

        case OpCode::ADD:
            std::cout << "============EXECUTANDO ADD============" << std::endl;
            if(instr.modo == ModoEnderecamento::IMEDIATO)
                running->acc += instr.operando_val;
            else
                running->acc += running->dados[instr.operando_str];
            std::cout << "Acc = " << running->acc << std::endl;
            break;

        case OpCode::SUB:
            std::cout << "============EXECUTANDO SUB============" << std::endl;
            if(instr.modo == ModoEnderecamento::IMEDIATO)
                running->acc -= instr.operando_val;
            else
                running->acc -= running->dados[instr.operando_str];
            std::cout << "Acc = " << running->acc << std::endl;
            break;

        // ----- Restante das instruções implementadas abaixo -----

        case OpCode::MULT:
            std::cout << "============EXECUTANDO MULT============" << std::endl;
            if (instr.modo == ModoEnderecamento::IMEDIATO)
                running->acc *= instr.operando_val;
            else
                running->acc *= running->dados[instr.operando_str];
            std::cout << "Acc = " << running->acc << std::endl;
            break;

        case OpCode::DIV:
            std::cout << "============EXECUTANDO DIV=============" << std::endl;
            { // Bloco para criar uma variável local 'divisor'
                int divisor = 0;
                if (instr.modo == ModoEnderecamento::IMEDIATO) {
                    divisor = instr.operando_val;
                } else {
                    divisor = running->dados[instr.operando_str];
                }

                if (divisor != 0) {
                    running->acc /= divisor;
                } else {
                    std::cout << "ERRO: Tentativa de divisao por zero!" << std::endl;
                    // Opcional: finalizar o processo por erro
                    // this->exit.push_back(this->running);
                    // this->running = nullptr;
                }
            }
            std::cout << "Acc = " << running->acc << std::endl;
            break;

        case OpCode::LOAD:
            std::cout << "============EXECUTANDO LOAD=============" << std::endl;
            if (instr.modo == ModoEnderecamento::IMEDIATO)
                running->acc = instr.operando_val;
            else
                running->acc = running->dados[instr.operando_str];
            std::cout << "Acc = " << running->acc << std::endl;
            break;

        case OpCode::STORE:
            std::cout << "============EXECUTANDO STORE============" << std::endl;
            if (instr.modo == ModoEnderecamento::DIRETO) {
                running->dados[instr.operando_str] = running->acc;
                std::cout << "Memoria[" << instr.operando_str << "] = " << running->acc << std::endl;
            } else {
                std::cout << "ERRO: Modo de enderecamento IMEDIATO invalido para STORE." << std::endl;
            }
            break;

        case OpCode::BRANY:
            std::cout << "============EXECUTANDO BRANY============" << std::endl;
            running->pc = instr.operando_val - 1; // -1 para compensar o pc++ no final
            std::cout << "Salto incondicional para PC = " << instr.operando_val << std::endl;
            break;

        case OpCode::BRPOS:
            std::cout << "============EXECUTANDO BRPOS============" << std::endl;
            if (running->acc > 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "Acc > 0. Saltando para PC = " << instr.operando_val << std::endl;
            } else {
                std::cout << "Acc <= 0. Nao saltou." << std::endl;
            }
            break;

        case OpCode::BRZERO:
            std::cout << "============EXECUTANDO BRZERO============" << std::endl;
            if (running->acc == 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "Acc == 0. Saltando para PC = " << instr.operando_val << std::endl;
            } else {
                std::cout << "Acc != 0. Nao saltou." << std::endl;
            }
            break;

        case OpCode::BRNEG:
            std::cout << "============EXECUTANDO BRNEG============" << std::endl;
            if (running->acc < 0) {
                running->pc = instr.operando_val - 1;
                std::cout << "Acc < 0. Saltando para PC = " << instr.operando_val << std::endl;
            } else {
                std::cout << "Acc >= 0. Nao saltou." << std::endl;
            }
            break;

        case OpCode::INVALIDO:
        default:
            std::cout << "ERRO: Instrucao invalida encontrada!" << std::endl;
            // this->exit.push_back(this->running);
            // this->running = nullptr;
            break;
    }

    // Incrementa o PC se o processo não terminou ou não saltou
    if (this->running != nullptr) {
        this->running->pc++;
    }
     // Avança para a próxima instrução
    return true; // Retorna true para indicar que o processo continua
}