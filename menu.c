#include "td.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int verificar_save(void) {
  FILE *fp = fopen(SAVE_FILE, "r");
  if (!fp) {
    return 0;
  }
  fclose(fp);
  return 1;
}

void mostrar_resumo_save(void) {
  FILE *fp = fopen(SAVE_FILE, "r");
  if (!fp) {
    printf("\n[!] Nenhum arquivo '%s' encontrado para carregar.\n", SAVE_FILE);
    return;
  }
  int round = 1, torre_vida = 3, qtd_inimigos = 3, qtd_torres = 3;
  char linha[256];
  system("clear");
  printf("\n--- Conteudo encontrado em %s ---\n", SAVE_FILE);
  while (fgets(linha, sizeof(linha), fp) != NULL) {
    printf("  %s", linha);
    sscanf(linha, "ROUND=%d", &round);
    sscanf(linha, "TORRE_PRINCIPAL_VIDA=%d", &torre_vida);
    sscanf(linha, "QTD_INIMIGOS=%d", &qtd_inimigos);
    sscanf(linha, "QTD_TORRES_MENORES=%d", &qtd_torres);
  }
  fclose(fp);

  printf("---------------------------------------\n");
  printf("Resumo: Round %d | Torre Principal: %d HP | Inimigos: %d | Torres "
         "Menores: %d\n",
         round, torre_vida, qtd_inimigos, qtd_torres);
  sleep(1);
}

int executar_jogo(int carregar_save) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("Erro ao criar processo (fork)");
    return 1;
  }
  if (pid == 0) {
    if (carregar_save) {
      execl("./jogo", "./jogo", "--load", (char *)NULL);
    } else {
      execl("./jogo", "./jogo", (char *)NULL);
    }
    perror(
        "Erro ao executar ./jogo (verifique se compilou jogo.c para ./jogo)");
    _exit(127);
  }
  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("Erro ao aguardar processo filho (waitpid)");
    return 1;
  }
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  return 1;
}

int main(void) {
  int opcao = 0;
  int rodou_com_load = 0;

  while (1) {
    system("clear");
    printf("\n========================================\n");
    printf("        TOWER DEFENSE - MENU PRINCIPAL    \n");
    printf("========================================\n");
    printf("1 - Entrar no jogo\n");
    printf("2 - Carregar jogo (%s)\n", SAVE_FILE);
    printf("3 - Sair\n");
    printf("Escolha uma opcao: ");
    if (scanf("%d", &opcao) != 1) {
      int c;
      while ((c = getchar()) != '\n' && c != EOF) {
      }
      printf("\n[!] Entrada invalida. Digite um numero de 1 a 3.\n");
      continue;
    }
    if (opcao == 1) {
      rodou_com_load = 0;
      int codigo_saida = executar_jogo(0);
      if (codigo_saida == 2) {
        printf("\n[i] Jogo salvo e retornado ao menu principal.\n");
      } else if (codigo_saida == 0) {
        printf("\n[i] Jogo finalizado. Retornando ao menu.\n");
      } else {
        printf("\n[!] ./jogo encerrou com codigo %d.\n", codigo_saida);
      }
    } else if (opcao == 2) {
      if (!verificar_save()) {
        printf(
            "\n[!] Arquivo '%s' nao existe. Inicie um jogo e salve primeiro.\n",
            SAVE_FILE);
        continue;
      }
      mostrar_resumo_save();
      rodou_com_load = 1;
      int codigo_saida = executar_jogo(1);
      if (codigo_saida == 2) {
        printf("\n[i] Progresso salvo em %s.\n", SAVE_FILE);
      } else if (codigo_saida == 10) {
        printf(
            "\n[i] Retorno especial do jogo (carregado anteriormente: %d).\n",
            rodou_com_load);
      }
    } else if (opcao == 3) {
      printf("\nEncerrando o programa. Ate mais!\n");
      return 0;
    } else {
      printf("\n[!] Opcao invalida. Escolha 1, 2 ou 3.\n");
    }
  }
  return 0;
}
