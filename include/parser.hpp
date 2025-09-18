#pragma once

#include <string>
#include <map>
#include "processo.hpp"

class Parser {
public:
    Processo parse(const std::string& nome_arquivo);

private:
    std::string trim(const std::string& s);
    OpCode string_to_opcode(const std::string& s);

    void encontra_lables(std::ifstream& arquivo, std::map<std::string, int>& tabelaDeLabels);
    void cria_estrutura(std::ifstream& arquivo, Processo& processo, const std::map<std::string, int>& tabelaDeLabels);
};