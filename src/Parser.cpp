#include "parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

Processo Parser::parse(const std::string& nome_arquivo) {
    Processo processo;
    std::map<std::string, int> tabelaDeLabels;
    std::ifstream arquivo(nome_arquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo " << nome_arquivo << std::endl;
        return processo;
    }

    encontra_lables(arquivo, tabelaDeLabels);
    arquivo.clear();
    arquivo.seekg(0, std::ios::beg);
    passo2_gerar_estruturas(arquivo, processo, tabelaDeLabels);

    return processo;
}

std::string Parser::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

OpCode Parser::string_to_opcode(const std::string& s) {
    std::string upper_s = s;
    std::transform(upper_s.begin(), upper_s.end(), upper_s.begin(), ::toupper);
    if (upper_s == "ADD") return OpCode::ADD;
    if (upper_s == "SUB") return OpCode::SUB;
    if (upper_s == "MULT") return OpCode::MULT;
    if (upper_s == "DIV") return OpCode::DIV;
    if (upper_s == "LOAD") return OpCode::LOAD;
    if (upper_s == "STORE") return OpCode::STORE;
    if (upper_s == "BRANY") return OpCode::BRANY;
    if (upper_s == "BRPOS") return OpCode::BRPOS;
    if (upper_s == "BRZERO") return OpCode::BRZERO;
    if (upper_s == "BRNEG") return OpCode::BRNEG;
    if (upper_s == "SYSCALL") return OpCode::SYSCALL;
    return OpCode::INVALIDO;
}

void Parser::encontra_lables(std::ifstream& arquivo, std::map<std::string, int>& tabelaDeLabels) {
    std::string linha;
    int numero_da_instrucao = 0;
    bool em_secao_codigo = false;

    while (std::getline(arquivo, linha)) {
        
        linha = trim(linha);

        size_t pos_comentario = linha.find("# ");
        if (pos_comentario != std::string::npos) {
            linha = linha.substr(0, pos_comentario);
        }
        std::cout<<"LINHA " << linha << std::endl;

        if (linha.empty()) continue;
        if (linha.rfind("#", 0) == 0) continue; // Ignora linhas que começam com #

        if (linha == ".code") { em_secao_codigo = true; continue; }
        if (linha == ".endcode") { em_secao_codigo = false; continue; }

        if (em_secao_codigo) {
            size_t pos_label = linha.find(':');
            if (pos_label != std::string::npos) {
                std::string label = trim(linha.substr(0, pos_label));
                tabelaDeLabels[label] = numero_da_instrucao;
                linha = trim(linha.substr(pos_label + 1));
            }
            if (!linha.empty()) {
                numero_da_instrucao++;
            }
        }
    }
}

void Parser::passo2_gerar_estruturas(std::ifstream& arquivo, Processo& processo, const std::map<std::string, int>& tabelaDeLabels) {
    std::string linha;
    bool em_secao_codigo = false;
    bool em_secao_dados = false;
    bool em_secao_escalonador = false;

    while (std::getline(arquivo, linha)) {
        size_t pos_comentario = linha.find("# ");
        if (pos_comentario != std::string::npos) {
            linha = linha.substr(0, pos_comentario);
        }

        linha = trim(linha);

        if (linha.empty()) continue;
        if (linha.rfind("#", 0) == 0) continue;

        if (linha == ".code") { em_secao_codigo = true; em_secao_dados = false; em_secao_escalonador = false; continue; }
        if (linha == ".endcode") { em_secao_codigo = false; continue; }
        if (linha == ".data") { em_secao_dados = true; em_secao_codigo = false; em_secao_escalonador = false; continue; }
        if (linha == ".enddata") { em_secao_dados = false; continue; }
        if (linha == ".scheduling") { em_secao_escalonador = true; em_secao_codigo = false; em_secao_dados = false; continue; }
        if (linha == ".endscheduling") { em_secao_escalonador = false; continue; }

        if (em_secao_codigo) {
            // Agora, esta busca por ':' é segura, pois a parte do comentário já foi removida.
            size_t pos_label = linha.find(':');
            if (pos_label != std::string::npos) {
                linha = trim(linha.substr(pos_label + 1));
            }
            if (linha.empty()) continue;

            std::stringstream ss(linha);
            std::string mnem, op;
            ss >> mnem >> op;

            Instrucao instr;
            instr.opcode = string_to_opcode(mnem);
            instr.operando_str = op;

            if (!op.empty() && op[0] == '#') {
                instr.modo = ModoEnderecamento::IMEDIATO;
                instr.operando_val = std::stoi(op.substr(1));
            } else {
                instr.modo = ModoEnderecamento::DIRETO;
                if (instr.opcode == OpCode::SYSCALL) {
                    if (!op.empty()) {
                        instr.operando_val = std::stoi(op);
                    } else {
                        instr.operando_val = 0;
                    }
                } else if (
                    instr.opcode == OpCode::BRANY  ||
                    instr.opcode == OpCode::BRPOS  ||
                    instr.opcode == OpCode::BRZERO ||
                    instr.opcode == OpCode::BRNEG
                ) {
                    if (tabelaDeLabels.count(op)) {
                        instr.operando_val = tabelaDeLabels.at(op);
                    } else {
                        instr.operando_val = 0; 
                    }
                } else {
                    instr.operando_val = 0; 
                }
            }
            processo.codigo.push_back(instr);
        } else if (em_secao_dados) {
            std::stringstream ss(linha);
            std::string nome_var;
            int valor_var;
            ss >> nome_var >> valor_var;
            if (!nome_var.empty()) {
                processo.dados[nome_var] = valor_var;
            }
        } else if (em_secao_escalonador) {
            std::stringstream ss(linha);
            std::string tipo;
            std::string valor;
            ss >> tipo >> valor;
            // std::cout<< tipo << std::endl;
            // std::cout<< valor << std::endl;
            // std::cout <<"================" << std::endl;

            if (tipo == "RR") { 
                processo.sched = Scheduling::RR;
                // Corrigindo a lógica de prioridade para usar o valor do enum
                //std::cout <<"here" << std::endl;
                processo.quantum = valor.empty() ? 1 : std::stoi(valor);
                processo.restante_quantum = processo.quantum;
            }
            if (tipo == "FCFS") {
                processo.sched = Scheduling::FCFS;
            }
            if (tipo == "arrival") {
                processo.arrival_time = std::stoi(valor);
            }
            if (tipo == "prio") {
                processo.prio = std::stoi(valor);
            }
        }
    }
}
