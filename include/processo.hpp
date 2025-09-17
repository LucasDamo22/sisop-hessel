#pragma once

#include <string>
#include <vector>
#include <map>

enum class OpCode {
    ADD, SUB, MULT, DIV,
    LOAD, STORE,
    BRANY, BRPOS, BRZERO, BRNEG,
    SYSCALL,
    INVALIDO
};

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

struct Instrucao {
    OpCode opcode;
    std::string operando_str;
    int operando_val;
    ModoEnderecamento modo;

    Instrucao() : opcode(OpCode::INVALIDO),  operando_val(0), modo(ModoEnderecamento::DIRETO) {}
};

std::string scheduling_to_string(Scheduling s);
std::string estado_to_string(Estado e);
std::string opcode_to_string(OpCode op);

class Processo {
public:
    int id;
    int pc;
    int acc;
    int arrival_time;
    int prio;
    int wait_time;
    Scheduling sched;
    Estado estado;
    
    std::vector<Instrucao> codigo;
    std::map<std::string, int> dados;

    void tostr() const;
    void imprimir_instrucao(size_t endereco) const;


    Processo() : id(0), pc(0), acc(0), wait_time(0), sched(Scheduling::FCFS), estado(Estado::NEW), prio(0x7fffffff) {}
};