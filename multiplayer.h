#include <stdbool.h>
#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include <pthread.h>
#include "baralho.h"
#include "jogador.h"

#define MAX_JOGADORES 5

typedef struct {
    JogadorInfo jogador_base;
    pthread_t thread;
    int vez;
    int id;
} JogadorMulti;

typedef struct {
    JogadorMulti jogadores_multi[MAX_JOGADORES];
    int num_jogadores_multi;
    int dealer_mao_multi[MAX_CARTAS];
    int dealer_cartas_multi;
} MesaMulti;

extern MesaMulti mesa_multi;

void multiplayer_iniciar(MesaMulti *mesa);
void multiplayer_rodada(MesaMulti *mesa);
void *thread_jogador_multi(void *arg);
void mostrar_rodada(JogadorMulti *j, int rodada);
void mostrar_tela_dealer(MesaMulti *mesa, bool mostrar_todas);
void mostrar_resultados(MesaMulti *mesa);
int calcular_pontos_blackjack(int *mao, int num_cartas);
const char* carta_valor_simples(int carta);
void jogar_multiplayer(MesaMulti *mesa);


#endif