#pragma once

#include "processo.hpp"
#include <queue>
#include <vector>

class CPU {
public:
    const int QUANTUM = 4;
    int pid;
    int elapsed_time;
    
    std::deque<Processo*> real_time;
    std::deque<Processo*> best_effort;
    std::vector<Processo*> newprocess;
    std::deque<Processo*> waiting;
    std::vector<Processo*> exit;
    Processo* running;
    
    std::vector<Processo> processos; 
    
    CPU();

    
    void boot();
    void executar();
    void escalonador(std::stringstream& logger);
    void spawn_proc();
    void insere_realtime(Processo *p1);
    void print_all_process();

private:
    bool executarInstrucao(std::stringstream& logger);
};