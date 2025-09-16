#pragma once

#include "processo.hpp" // Inclui a definição de Processo
#include <queue>
#include <vector>

// Declaração da classe CPU
class CPU {
public:
    const int QUANTUM = 4; // Quantum para processos Round Robin
    int pid;
    // Filas para os processos prontos (guardam ponteiros)
    std::vector<Processo*> real_time;
    std::vector<Processo*> best_effort;
    std::vector<Processo*> newprocess;
    std::vector<Processo*> waiting;
    std::vector<Processo*> exit;
    Processo* running;
    
    std::vector<Processo> processos; 
    
    // Construtor
    CPU();

    // Métodos públicos
    void boot();
    void executar();
    void spawn_proc();

private:
    // Método auxiliar privado para executar uma única instrução
    bool executarInstrucao(Processo* p);
};