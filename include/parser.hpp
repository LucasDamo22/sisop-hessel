#pragma once

#include <string>
#include <map>
#include "processo.hpp" // Inclui as definições de Processo, Instrucao, etc.

class Parser {
public:
    // O único método público, que lê um arquivo e retorna um objeto Processo
    Processo parse(const std::string& nome_arquivo);

private:
    // Funções auxiliares usadas internamente pelo parser
    std::string trim(const std::string& s);
    OpCode string_to_opcode(const std::string& s);

    // Lógica dos dois passos do parser
    void encontra_lables(std::ifstream& arquivo, std::map<std::string, int>& tabelaDeLabels);
    void passo2_gerar_estruturas(std::ifstream& arquivo, Processo& processo, const std::map<std::string, int>& tabelaDeLabels);
};