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
    FCFS,
    INDEFINIDO
};

enum class Estado {
    NEW,
    READY,
    RUNNING,
    WAITING,
    EXIT
};

enum class Prioridade {
    ALTA,
    BAIXA
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
    int deadline;
    Scheduling sched;
    Estado estado;
    Prioridade prio;
    
    // Segmentos de memória do processo
    std::vector<Instrucao> codigo;
    std::map<std::string, int> dados;

    // Construtor padrão
    Processo() : id(0), pc(0), acc(0), sched(Scheduling::INDEFINIDO), estado(Estado::NEW), deadline(0xffffffff), prio(Prioridade::BAIXA) {}
};