#include <iostream>
#include <string>
#include "utils/InputParser.h"

/*
 * 1. Wlacza sie 1 klient, dostaje null jako adres nastepnego
 * 2. Wlacza sie 2 klient i dostaje port poprzedniego
 *
 * Potrzebujemy sie zabezpieczyc przed sytuacja gdzie stacja koncowa/poczatkowa umarla. Moze jakis TTL?
 *
 *
 * Token służy więc do utworzenia ramki danych. Nadająca stacja zmienia sekwencję SOF, dodaje potrzebne dane, adresuje je
 * i umieszcza z powrotem w sieci. Jeśli stacja nie chce nadawać, może po prostu z powrotem umieścić token w sieci
 * – wtedy otrzyma go kolejna stacja. Gdy ramka dotrze do miejsca przeznaczenia, urządzenie odbierające nie wyciąga ramki z sieci,
 * lecz po prostu kopiuje jej zawartość do bufora w celu dalszego wewnętrznego przetwarzania. W oryginalnej ramce zmieniany jest
 * bit pola sterowania dostępem, co informuje nadawcę, że ramka została odebrana. Potem ramka kontynuuje swoją podróż przez pierścień,
 * dopóki nie powróci do urządzenia, które ją wysłało. Gdy urządzenie ją odbierze, uznaje się, że transmisja zakończyła się sukcesem;
 * zawartość ramki jest kasowana, a sama ramka jest z powrotem przekształcana w token.
 */

int main(int argc, char *argv[]) {
    std::cout << "Hello, World!" << std::endl;
    Input input = InputParser::parseArguments(argc, argv);

    std::cout << input.protocol << std::endl;
    std::cout << input.neighbourIpAddess << std::endl;
    std::cout << input.listeningPort << std::endl;
    std::cout << input.id << std::endl;
    std::cout << input.doesHaveToken << std::endl;


    return 0;
}