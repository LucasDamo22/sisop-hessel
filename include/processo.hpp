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

struct Processo {
    int id = -1;
    int pc = 0;
    int acc = 0;
    Scheduling sched = Scheduling::FCFS;
    int prio = 1;
    int quantum = 1;
    int restante_quantum = 1;
    int arrival_time = 0;
    int wait_time = -1;
    Estado estado = Estado::NEW;
    std::vector<Instrucao> codigo;
    std::map<std::string,int> dados;
    Processo() : id(-1), pc(0), acc(0), sched(Scheduling::FCFS), estado(Estado::NEW),prio(1), quantum(1), restante_quantum(1), arrival_time(0), wait_time(-1) {}
};
