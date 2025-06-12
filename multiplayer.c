#include "jogo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "multiplayer.h"

MesaMulti mesa_multi;

void limpar_tela() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

const char* carta_valor_simples(int carta) {
    static const char* valores[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    return valores[carta - 1];
}

int calcular_pontos_blackjack(int *mao, int num_cartas) {
    int total = 0, ases = 0;
    for (int i = 0; i < num_cartas; i++) {
        if (mao[i] == 1) {
            total += 11;
            ases++;
        } else if (mao[i] >= 10) {
            total += 10;
        } else {
            total += mao[i];
        }
    }
    while (total > 21 && ases > 0) {
        total -= 10;
        ases--;
    }
    return total;
}

void mostrar_rodada(JogadorMulti *j, int rodada) {
    limpar_tela();
    printf("========= RODADA %d =========\n", rodada);
    printf("========================================\n");
    printf("Jogador: %s\n", j->jogador_base.nome);
    printf("Saldo: %.2f | Aposta: %.2f\n", j->jogador_base.saldo, j->jogador_base.aposta);
    printf("========================================\n");
    printf("Cartas: ");
    for (int i = 0; i < j->jogador_base.num_cartas; i++) {
        printf("[%s] ", carta_valor_simples(j->jogador_base.cartas[i]));
    }
    int total = calcular_pontos_blackjack(j->jogador_base.cartas, j->jogador_base.num_cartas);
    printf("\nTotal: %d\n", total);
    if (total == 21 && j->jogador_base.num_cartas == 2) {
        printf("BLACKJACK!\n");
    } else if (total > 21) {
        printf("ESTOURO!\n");
    }
    printf("========================================\n");
}

void mostrar_tela_dealer(MesaMulti *mesa, bool mostrar_todas) {
    limpar_tela();
    printf("========= DEALER =========\n");
    printf("========================================\n");
    printf("Cartas: ");
    for (int i = 0; i < mesa->dealer_cartas_multi; i++) {
        if (i == 0 && !mostrar_todas) {
            printf("[?] ");
        } else {
            printf("[%s] ", carta_valor_simples(mesa->dealer_mao_multi[i]));
        }
    }
    if (mostrar_todas) {
        int total = calcular_pontos_blackjack(mesa->dealer_mao_multi, mesa->dealer_cartas_multi);
        printf("\nTotal: %d\n", total);
    }
    printf("========================================\n");
    sleep(1);
}


void *thread_jogador_multi(void *arg) {
    JogadorMulti *j = (JogadorMulti *)arg;
    
    // Aposta inicial
    do {
        mostrar_rodada(j, 1);
        printf("\n%s, faça sua aposta (saldo: %.2f): ", j->jogador_base.nome, j->jogador_base.saldo);
        scanf("%lf", &j->jogador_base.aposta);
                if (j->jogador_base.aposta == j->jogador_base.saldo) mostrar_tela_allin();
getchar();
        
        if (j->jogador_base.aposta > j->jogador_base.saldo) {
            printf("Saldo insuficiente!\n");
            sleep(1);
        }
    } while (j->jogador_base.aposta > j->jogador_base.saldo);
    
    // Turno do jogador
    j->vez = true;
    while (j->vez) {
        mostrar_rodada(j, 1);
        printf("\nDeseja receber uma carta? (s/n): ");
        char acao = getchar();
        getchar();
        
        if (tolower(acao) == 's') {
            j->jogador_base.cartas[j->jogador_base.num_cartas++] = comprar_carta();
            if (calcular_pontos_blackjack(j->jogador_base.cartas, j->jogador_base.num_cartas) > 21) {
                j->vez = false;
            }
        } else {
            j->vez = false;
        }
    }
    return NULL;
}

void multiplayer_iniciar(MesaMulti *mesa) {
    // Reinicializa a mesa
    memset(mesa, 0, sizeof(MesaMulti));
    inicializar_baralho();
    
    printf("\n=== CONFIGURAÇÃO DO MULTIPLAYER ===\n");
    printf("Quantos jogadores (1-%d)? ", MAX_JOGADORES);
    
    // Validação do número de jogadores
    while (1) {
        if (scanf("%d", &mesa->num_jogadores_multi) != 1 || 
            mesa->num_jogadores_multi < 1 || 
            mesa->num_jogadores_multi > MAX_JOGADORES) {
            printf("Número inválido! Digite entre 1 e %d: ", MAX_JOGADORES);
            while (getchar() != '\n');
            continue;
        }
        break;
    }
    getchar();
    
    // Configuração de cada jogador
    for (int i = 0; i < mesa->num_jogadores_multi; i++) {
        printf("\n--- Jogador %d ---\n", i+1);
        
        printf("Nome: ");
        fgets(mesa->jogadores_multi[i].jogador_base.nome, 50, stdin);
        mesa->jogadores_multi[i].jogador_base.nome[strcspn(mesa->jogadores_multi[i].jogador_base.nome, "\n")] = '\0';
        
        printf("Saldo inicial (mínimo 5): ");
        while (scanf("%lf", &mesa->jogadores_multi[i].jogador_base.saldo) != 1 || 
               mesa->jogadores_multi[i].jogador_base.saldo < 5) {
            printf("Valor inválido! Digite novamente: ");
            while (getchar() != '\n');
        }
        getchar();
        
        // Inicializa dados do jogador
        mesa->jogadores_multi[i].id = i;
        mesa->jogadores_multi[i].vez = false;
        mesa->jogadores_multi[i].jogador_base.num_cartas = 0;
        mesa->jogadores_multi[i].jogador_base.aposta = 0;
    }
}

void multiplayer_rodada(MesaMulti *mesa) {
    mostrar_tela_dealer(mesa, true);
    printf("\nO dealer está jogando...\n");
    sleep(1);
    
    while (calcular_pontos_blackjack(mesa->dealer_mao_multi, mesa->dealer_cartas_multi) < 17) {
        mesa->dealer_mao_multi[mesa->dealer_cartas_multi++] = comprar_carta();
        mostrar_tela_dealer(mesa, true);
        sleep(1);
    }
}

void mostrar_resultados(MesaMulti *mesa) {
    limpar_tela();
    printf("========= RESULTADOS FINAIS =========\n");
    printf("========================================\n");
    
    int pontos_dealer = calcular_pontos_blackjack(mesa->dealer_mao_multi, mesa->dealer_cartas_multi);
    mostrar_tela_dealer(mesa, true);
    
    for (int i = 0; i < mesa->num_jogadores_multi; i++) {
        int pontos_jogador = calcular_pontos_blackjack(
            mesa->jogadores_multi[i].jogador_base.cartas,
            mesa->jogadores_multi[i].jogador_base.num_cartas
        );
        
        printf("\n%s: %d pontos - ", mesa->jogadores_multi[i].jogador_base.nome, pontos_jogador);
        
        if (pontos_jogador > 21) {
            printf("Perdeu!");
        } else if (pontos_dealer > 21 || pontos_jogador > pontos_dealer) {
            mesa->jogadores_multi[i].jogador_base.saldo += mesa->jogadores_multi[i].jogador_base.aposta * 2;
            printf("Venceu! (+%.2f)", mesa->jogadores_multi[i].jogador_base.aposta * 2);
        } else if (pontos_jogador == pontos_dealer) {
            mesa->jogadores_multi[i].jogador_base.saldo += mesa->jogadores_multi[i].jogador_base.aposta;
            printf("Empate! (+%.2f)", mesa->jogadores_multi[i].jogador_base.aposta);
        } else {
            printf("Perdeu!");
        }
    }
    
    printf("\n\n========================================\n");
    printf("Pressione Enter para voltar ao menu...");
    getchar();
    getchar(); // Dupla leitura para garantir
}

void jogar_multiplayer(MesaMulti *mesa) {
    system("clear");
    printf("\n=== INICIANDO MODO MULTIPLAYER ===\n");
    sleep(1);
    
    // 1. Distribuição inicial de cartas
    for (int i = 0; i < mesa->num_jogadores_multi; i++) {
        mesa->jogadores_multi[i].jogador_base.cartas[0] = comprar_carta();
        mesa->jogadores_multi[i].jogador_base.cartas[1] = comprar_carta();
        mesa->jogadores_multi[i].jogador_base.num_cartas = 2;
        mesa->jogadores_multi[i].vez = true;
    }
    
    // 2. Cartas do dealer
    mesa->dealer_mao_multi[0] = comprar_carta();
    mesa->dealer_mao_multi[1] = comprar_carta();
    mesa->dealer_cartas_multi = 2;
    
    // 3. Mostra a mesa inicial
    mostrar_tela_dealer(mesa, false);
    sleep(1);
    
    // 4. Turnos dos jogadores
    for (int i = 0; i < mesa->num_jogadores_multi; i++) {
        printf("\n=== VEZ DE %s ===\n", mesa->jogadores_multi[i].jogador_base.nome);
        thread_jogador_multi(&mesa->jogadores_multi[i]);
    }
    
    // 5. Turno do dealer
    printf("\n=== TURNO DO DEALER ===\n");
    multiplayer_rodada(mesa);
    
    // 6. Resultados finais
    mostrar_resultados(mesa);
}
