

#include "processo.hpp" 
#include <iostream>     


std::string scheduling_to_string(Scheduling s) {
    switch (s) {
        case Scheduling::RR:   return "RR";
        case Scheduling::FCFS: return "FCFS";
        default:               return "Desconhecido";
    }
}

std::string estado_to_string(Estado e) {
    switch (e) {
        case Estado::NEW:     return "Novo";
        case Estado::READY:   return "Pronto";
        case Estado::RUNNING: return "Executando";
        case Estado::WAITING: return "Aguardando";
        case Estado::EXIT:    return "Finalizado";
        default:              return "Desconhecido";
    }
}



std::string opcode_to_string(OpCode op) {
    switch (op) {
        case OpCode::ADD:       return "ADD";
        case OpCode::SUB:       return "SUB";
        case OpCode::MULT:      return "MULT";
        case OpCode::DIV:       return "DIV";
        case OpCode::LOAD:      return "LOAD";
        case OpCode::STORE:     return "STORE";
        case OpCode::BRANY:     return "BRANY";
        case OpCode::BRPOS:     return "BRPOS";
        case OpCode::BRZERO:    return "BRZERO";
        case OpCode::BRNEG:     return "BRNEG";
        case OpCode::SYSCALL:   return "SYSCALL";
        case OpCode::INVALIDO:  return "INVALIDO";
        default:                return "OP_DESCONHECIDO";
    }
}

void Processo::imprimir_instrucao(size_t endereco) const {

    const auto& instr = codigo[endereco];

    std::cout << "Endereco " << endereco << ": "
              << "OpCode(" << opcode_to_string(instr.opcode) << "), "
              << "OperandoStr(" << instr.operando_str << "), "
              << "OperandoVal(" << instr.operando_val << "), "
              << "Modo(" << (instr.modo == ModoEnderecamento::DIRETO ? "DIRETO" : "IMEDIATO") << ")"
              << std::endl;
}


void Processo::tostr() const {
    std::cout << "--- Detalhes do Processo ID: " << this->id << " ---" << std::endl;
    std::cout << "Escalonamento: " << scheduling_to_string(this->sched) << std::endl;
    std::cout << "Estado: " << estado_to_string(this->estado) << std::endl;
    std::cout << "Prioridade: " << this->prio << std::endl;
    std::cout << "Arrival: " << this->arrival_time << std::endl;
    std::cout << "Burst: " << this->burst_time << std::endl;
    
    std::cout << "\n[SEGMENTO DE CODIGO]" << std::endl;
    for (size_t i = 0; i < codigo.size(); ++i) {
        imprimir_instrucao(i);
    }
    
    std::cout << "\n[SEGMENTO DE DADOS]" << std::endl;
    for (const auto& par : dados) {
        std::cout << "Variavel '" << par.first << "' = " << par.second << std::endl;
    }
}