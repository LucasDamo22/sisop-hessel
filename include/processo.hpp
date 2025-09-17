#pragma once // Evita que o header seja incluído múltiplas vezes

#include <string>
#include <vector>
#include <map>

// Enum para todos os mnemônicos possíveis
enum class OpCode {
    ADD, SUB, MULT, DIV,
    LOAD, STORE,
    BRANY, BRPOS, BRZERO, BRNEG,
    SYSCALL,
    INVALIDO
};

// Enum para os modos de endereçamento
enum class ModoEnderecamento {
    DIRETO,
    IMEDIATO
};

enum class Scheduling {
    RR,
    FCFS
};

enum class Estado {
    NEW,
    READY,
    RUNNING,
    WAITING,
    EXIT
};



// Estrutura para representar uma única instrução traduzida
struct Instrucao {
    OpCode opcode;
    std::string operando_str;
    int operando_val;
    ModoEnderecamento modo;

    Instrucao() : opcode(OpCode::INVALIDO),  operando_val(0), modo(ModoEnderecamento::DIRETO) {}
};

// Classe que representa o Processo, contendo seu código, dados e PCB
class Processo {
public:
    // PCB - Process Control Block
    int id;
    int pc;
    int acc;
    int arrival_time;
    int prio;
    int burst_time;
    Scheduling sched;
    Estado estado;
    
    
    // Segmentos de memória do processo
    std::vector<Instrucao> codigo;
    std::map<std::string, int> dados;

    void tostr() const;
    void imprimir_instrucao(size_t endereco) const;


    // Construtor padrão
    Processo() : id(0), pc(0), acc(0), sched(Scheduling::FCFS), estado(Estado::NEW), prio(0x7fffffff) {}
};