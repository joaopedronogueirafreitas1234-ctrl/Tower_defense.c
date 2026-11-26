#include "td.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void dormir_ms(int ms) {
  if (ms <= 0) {
    return;
  }
  unsigned int seg = (unsigned int)(ms / 1000);
  if (seg == 0) {
    seg = 1;
  }
  sleep(seg);
}

void limpar_tela_animacao(void) {
  printf("\033[H\033[J");
  fflush(stdout);
}

void limpar_buffer(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

int ler_inteiro(const char *prompt, int min_val, int max_val) {
  int val;
  while (1) {
    printf("%s", prompt);
    if (scanf("%d", &val) == 1 && val >= min_val && val <= max_val) {
      limpar_buffer();
      return val;
    }
    limpar_buffer();
    printf("[!] Digite um numero valido entre %d e %d.\n", min_val, max_val);
  }
}

void liberar_tudo(Inimigo **inimigos, TorreMenor **torres_menores,
                  TorrePrincipal **torre_principal) {
  if (inimigos && *inimigos) {
    free(*inimigos);
    *inimigos = NULL;
  }
  if (torres_menores && *torres_menores) {
    free(*torres_menores);
    *torres_menores = NULL;
  }
  if (torre_principal && *torre_principal) {
    free(*torre_principal);
    *torre_principal = NULL;
  }
}

int alocar_ou_realocar_arrays(Inimigo **inimigos, int qtd_inimigos,
                              TorreMenor **torres_menores, int qtd_torres,
                              int eh_primeiro_round) {
  if (eh_primeiro_round) {
    Inimigo *novo_ini =
        (Inimigo *)malloc((size_t)qtd_inimigos * sizeof(Inimigo));
    if (!novo_ini) {
      perror("Falha no malloc para inimigos");
      return 0;
    }
    *inimigos = novo_ini;

    TorreMenor *novo_tor =
        (TorreMenor *)malloc((size_t)qtd_torres * sizeof(TorreMenor));
    if (!novo_tor) {
      perror("Falha no malloc para torres_menores");
      free(*inimigos);
      *inimigos = NULL;
      return 0;
    }
    *torres_menores = novo_tor;
  } else {
    Inimigo *novo_ini =
        (Inimigo *)realloc(*inimigos, (size_t)qtd_inimigos * sizeof(Inimigo));
    if (!novo_ini) {
      perror("Falha no realloc para inimigos");
      return 0;
    }
    *inimigos = novo_ini;

    TorreMenor *novo_tor = (TorreMenor *)realloc(
        *torres_menores, (size_t)qtd_torres * sizeof(TorreMenor));
    if (!novo_tor) {
      perror("Falha no realloc para torres_menores");
      return 0;
    }
    *torres_menores = novo_tor;
  }
  return 1;
}

void configurar_entidades_round(int round, Inimigo *inimigos, int qtd_inimigos,
                                TorreMenor *torres_menores, int qtd_torres,
                                int preservar_posicoes_torres) {
  int vida_base_inimigo = 25 + (round - 1) * 20;
  for (int i = 0; i < qtd_inimigos; i++) {
    inimigos[i].vida = vida_base_inimigo;
    inimigos[i].posicao = -(i / NUM_LINHAS);
    inimigos[i].linha = i % NUM_LINHAS;
    inimigos[i].qtd = 1;
  }
  for (int i = 0; i < qtd_torres; i++) {
    torres_menores[i].dano = 10 + (round - 1) * 5;
    torres_menores[i].alcance = 2;
    torres_menores[i].quantidade = 1;
    if (!preservar_posicoes_torres) {
      int pos_padrao = (i * 3) % (MAPA_TAM - 1);
      torres_menores[i].posicao = pos_padrao;
      torres_menores[i].linha = i % NUM_LINHAS;
    } else if (torres_menores[i].posicao < 0 ||
               torres_menores[i].posicao >= MAPA_TAM - 1 ||
               torres_menores[i].linha < 0 ||
               torres_menores[i].linha >= NUM_LINHAS) {
      torres_menores[i].posicao = (i * 3) % (MAPA_TAM - 1);
      torres_menores[i].linha = i % NUM_LINHAS;
    }
  }
}

int salvar_jogo(const char *caminho, int round_atual, int qtd_inimigos_base,
                int qtd_torres_base, const Inimigo *inimigos,
                const TorreMenor *torres_menores,
                const TorrePrincipal *torre_principal) {
  FILE *fp = fopen(caminho, "w");
  if (!fp) {
    perror("Erro ao abrir arquivo para salvar");
    return 0;
  }
  fprintf(fp, "ROUND=%d\n", round_atual);
  fprintf(fp, "TORRE_PRINCIPAL_VIDA=%d\n", torre_principal->vida);
  fprintf(fp, "QTD_INIMIGOS=%d\n", qtd_inimigos_base);
  fprintf(fp, "QTD_TORRES_MENORES=%d\n", qtd_torres_base);
  for (int i = 0; i < qtd_inimigos_base; i++) {
    fprintf(fp, "INIMIGO %d %d %d %d\n", inimigos[i].vida, inimigos[i].posicao,
            inimigos[i].linha, inimigos[i].qtd);
  }
  for (int i = 0; i < qtd_torres_base; i++) {
    fprintf(fp, "TORRE %d %d %d %d %d\n", torres_menores[i].dano,
            torres_menores[i].posicao, torres_menores[i].linha,
            torres_menores[i].alcance, torres_menores[i].quantidade);
  }
  fclose(fp);
  return 1;
}

int carregar_jogo(const char *caminho, int *round_atual, int *qtd_inimigos_base,
                  int *qtd_torres_base, Inimigo **inimigos,
                  TorreMenor **torres_menores,
                  TorrePrincipal *torre_principal) {
  FILE *fp = fopen(caminho, "r");
  if (!fp) {
    perror("Erro ao abrir arquivo de save");
    return 0;
  }
  int r = 1, hp_tp = 3, q_ini = 6, q_tor = 5;
  char chave[64];
  while (fscanf(fp, "%63s", chave) == 1) {
    if (strncmp(chave, "ROUND=", 6) == 0) {
      sscanf(chave + 6, "%d", &r);
    } else if (strncmp(chave, "TORRE_PRINCIPAL_VIDA=", 21) == 0) {
      sscanf(chave + 21, "%d", &hp_tp);
    } else if (strncmp(chave, "QTD_INIMIGOS=", 13) == 0) {
      sscanf(chave + 13, "%d", &q_ini);
    } else if (strncmp(chave, "QTD_TORRES_MENORES=", 19) == 0) {
      sscanf(chave + 19, "%d", &q_tor);
    }
  }
  if (r < 1)
    r = 1;
  if (r > TOTAL_ROUNDS)
    r = TOTAL_ROUNDS;
  if (q_ini < 1)
    q_ini = 6;
  if (q_tor < 1)
    q_tor = 5;

  if (!alocar_ou_realocar_arrays(inimigos, q_ini, torres_menores, q_tor, 1)) {
    fclose(fp);
    return 0;
  }
  torre_principal->vida = hp_tp;
  configurar_entidades_round(r, *inimigos, q_ini, *torres_menores, q_tor, 0);

  rewind(fp);
  int idx_ini = 0, idx_tor = 0;
  char linha_txt[256];
  while (fgets(linha_txt, sizeof(linha_txt), fp) != NULL) {
    int v, p, l, q, d, a;
    if (sscanf(linha_txt, "INIMIGO %d %d %d %d", &v, &p, &l, &q) == 4 &&
        idx_ini < q_ini) {
      (*inimigos)[idx_ini].vida = v;
      (*inimigos)[idx_ini].posicao = p;
      (*inimigos)[idx_ini].linha = (l >= 0 && l < NUM_LINHAS) ? l : 0;
      (*inimigos)[idx_ini].qtd = q;
      idx_ini++;
    } else if (sscanf(linha_txt, "TORRE %d %d %d %d %d", &d, &p, &l, &a, &q) ==
                   5 &&
               idx_tor < q_tor) {
      (*torres_menores)[idx_tor].dano = d;
      (*torres_menores)[idx_tor].posicao = p;
      (*torres_menores)[idx_tor].linha = (l >= 0 && l < NUM_LINHAS) ? l : 0;
      (*torres_menores)[idx_tor].alcance = a;
      (*torres_menores)[idx_tor].quantidade = q;
      idx_tor++;
    }
  }
  fclose(fp);

  *round_atual = r;
  *qtd_inimigos_base = q_ini;
  *qtd_torres_base = q_tor;
  return 1;
}

void imprimir_mapa_frame(const Inimigo *inimigos, int qtd_inimigos,
                         const TorreMenor *torres_menores, int qtd_torres,
                         const TorrePrincipal *torre_principal,
                         const int tiros_na_pos[NUM_LINHAS][MAPA_TAM],
                         int passo) {
  printf("\nMapa (%d linhas | 0 -> %d | Torre Principal na pos %d [HP: %d])",
         NUM_LINHAS, MAPA_TAM - 1, MAPA_TAM - 1, torre_principal->vida);
  if (passo > 0) {
    printf(" | Frame %d", passo);
  }
  printf("\nPos:  ");
  for (int i = 0; i < MAPA_TAM; i++) {
    printf("%2d ", i);
  }

  for (int l = 0; l < NUM_LINHAS; l++) {
    printf("\nL%d-T: ", l);
    for (int i = 0; i < MAPA_TAM; i++) {
      if (i == MAPA_TAM - 1) {
        printf("   ");
        continue;
      }
      int tem_tor = 0;
      int atirou = (tiros_na_pos != NULL && tiros_na_pos[l][i]) ? 1 : 0;
      for (int t = 0; t < qtd_torres; t++) {
        if (torres_menores[t].linha == l && torres_menores[t].posicao == i) {
          tem_tor = 1;
          break;
        }
      }
      if (atirou) {
        printf(" ! ");
      } else if (tem_tor) {
        printf(" T ");
      } else {
        printf("   ");
      }
    }

    printf("\nL%d-C: ", l);
    for (int i = 0; i < MAPA_TAM; i++) {
      if (i == MAPA_TAM - 1) {
        printf("[P]");
        continue;
      }
      int tem_ini = 0;
      for (int k = 0; k < qtd_inimigos; k++) {
        if (inimigos[k].vida > 0 && inimigos[k].linha == l &&
            inimigos[k].posicao == i) {
          tem_ini = 1;
          break;
        }
      }
      if (tem_ini) {
        printf(" E ");
      } else {
        printf(" . ");
      }
    }
  }
  printf("\nLegenda: Lx-T=Torres da linha x | Lx-C=Caminho da linha x | "
         "T=Torre | !=Tiro | E=Inimigo | [P]=Torre Principal\n");
}

int menu_e_executar_round(int round, int *qtd_inimigos_base,
                          int *qtd_torres_base, Inimigo **inimigos,
                          TorreMenor **torres_menores,
                          TorrePrincipal *torre_principal) {
  int escolha = 0;
  while (1) {
    system("clear");
    imprimir_mapa_frame(*inimigos, *qtd_inimigos_base, *torres_menores,
                        *qtd_torres_base, torre_principal, NULL, 0);
    printf("\n--- ROUND %d / %d ---\n", round, TOTAL_ROUNDS);
    printf("Torres menores ativas: %d | Inimigos do round: %d\n",
           *qtd_torres_base, *qtd_inimigos_base);
    printf("1 - Posicionar / mover torres menores\n");
    printf("2 - Iniciar onda do round\n");
    printf("3 - Salvar e voltar ao menu principal\n");
    escolha = ler_inteiro("Escolha: ", 1, 3);
    if (escolha == 1) {
      for (int t = 0; t < *qtd_torres_base; t++) {
        printf(
            "Torre menor #%d (dano %d, alcance %d, linha atual %d, pos atual "
            "%d)\n",
            t + 1, (*torres_menores)[t].dano, (*torres_menores)[t].alcance,
            (*torres_menores)[t].linha, (*torres_menores)[t].posicao);
        (*torres_menores)[t].linha =
            ler_inteiro("  Nova linha (0 a 2): ", 0, NUM_LINHAS - 1);
        (*torres_menores)[t].posicao =
            ler_inteiro("  Nova posicao (0 a 18): ", 0, MAPA_TAM - 2);
      }
    } else if (escolha == 2) {
      break;
    } else if (escolha == 3) {
      if (salvar_jogo(SAVE_FILE, round, *qtd_inimigos_base, *qtd_torres_base,
                      *inimigos, *torres_menores, torre_principal)) {
        printf("[OK] Jogo salvo em '%s'.\n", SAVE_FILE);
      }
      return 2;
    }
  }

  int pos_torre_principal = MAPA_TAM - 1;
  int passos = 0;
  int max_passos = MAPA_TAM + (*qtd_inimigos_base) + 5;
  int tiros_na_pos[NUM_LINHAS][MAPA_TAM];

  while (passos < max_passos && torre_principal->vida > 0) {
    passos++;
    memset(tiros_na_pos, 0, sizeof(tiros_na_pos));

    for (int i = 0; i < *qtd_inimigos_base; i++) {
      if ((*inimigos)[i].vida > 0) {
        (*inimigos)[i].posicao += 1;
      }
    }

    for (int t = 0; t < *qtd_torres_base; t++) {
      int p_tor = (*torres_menores)[t].posicao;
      int l_tor = (*torres_menores)[t].linha;
      int alc = (*torres_menores)[t].alcance;
      int dano = (*torres_menores)[t].dano;
      for (int i = 0; i < *qtd_inimigos_base; i++) {
        if ((*inimigos)[i].vida > 0 && (*inimigos)[i].linha == l_tor &&
            (*inimigos)[i].posicao >= 0) {
          int dist = (*inimigos)[i].posicao - p_tor;
          if (dist < 0)
            dist = -dist;
          if (dist <= alc) {
            (*inimigos)[i].vida -= dano;
            if (p_tor >= 0 && p_tor < MAPA_TAM && l_tor >= 0 &&
                l_tor < NUM_LINHAS) {
              tiros_na_pos[l_tor][p_tor] = 1;
            }
            if ((*inimigos)[i].vida <= 0) {
              (*inimigos)[i].vida = 0;
            }
            break;
          }
        }
      }
    }

    int atingiu_principal = 0;
    for (int i = 0; i < *qtd_inimigos_base; i++) {
      if ((*inimigos)[i].vida > 0 &&
          (*inimigos)[i].posicao >= pos_torre_principal) {
        torre_principal->vida -= 1;
        (*inimigos)[i].vida = 0;
        atingiu_principal = 1;
        if (torre_principal->vida <= 0) {
          torre_principal->vida = 0;
          break;
        }
      }
    }

    int vivos = 0;
    for (int i = 0; i < *qtd_inimigos_base; i++) {
      if ((*inimigos)[i].vida > 0) {
        vivos++;
      }
    }

    limpar_tela_animacao();
    imprimir_mapa_frame(*inimigos, *qtd_inimigos_base, *torres_menores,
                        *qtd_torres_base, torre_principal, tiros_na_pos,
                        passos);
    printf("Status da onda: Inimigos vivos = %d | HP Torre Principal = %d\n",
           vivos, torre_principal->vida);
    if (atingiu_principal) {
      printf("[!] Um inimigo alcancou a Torre Principal!\n");
    }
    fflush(stdout);
    dormir_ms(DELAY_MS);

    if (torre_principal->vida <= 0) {
      return 0;
    }

    int restam_vivos_ou_entrando = 0;
    for (int i = 0; i < *qtd_inimigos_base; i++) {
      if ((*inimigos)[i].vida > 0 &&
          (*inimigos)[i].posicao < pos_torre_principal) {
        restam_vivos_ou_entrando = 1;
        break;
      }
    }
    if (!restam_vivos_ou_entrando) {
      break;
    }
  }
  return 1;
}

int main(int argc, char **argv) {
  int modo_carregar = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--load") == 0) {
      modo_carregar = 1;
    }
  }

  TorrePrincipal *torre_principal =
      (TorrePrincipal *)malloc(sizeof(TorrePrincipal));
  if (!torre_principal) {
    perror("Falha no malloc para torre_principal");
    return 1;
  }
  torre_principal->vida = 3;

  Inimigo *inimigos = NULL;
  TorreMenor *torres_menores = NULL;
  int qtd_inimigos_base = 6;
  int qtd_torres_base = 5;
  int round_inicial = 1;

  if (modo_carregar) {
    if (!carregar_jogo(SAVE_FILE, &round_inicial, &qtd_inimigos_base,
                       &qtd_torres_base, &inimigos, &torres_menores,
                       torre_principal)) {
      printf("[!] Falha ao carregar save. Iniciando novo jogo do Round 1.\n");
      round_inicial = 1;
      qtd_inimigos_base = 6;
      qtd_torres_base = 5;
      torre_principal->vida = 3;
      if (!alocar_ou_realocar_arrays(&inimigos, qtd_inimigos_base,
                                     &torres_menores, qtd_torres_base, 1)) {
        liberar_tudo(&inimigos, &torres_menores, &torre_principal);
        return 1;
      }
      configurar_entidades_round(1, inimigos, qtd_inimigos_base, torres_menores,
                                 qtd_torres_base, 0);
    }
  } else {
    if (!alocar_ou_realocar_arrays(&inimigos, qtd_inimigos_base,
                                   &torres_menores, qtd_torres_base, 1)) {
      liberar_tudo(&inimigos, &torres_menores, &torre_principal);
      return 1;
    }
    configurar_entidades_round(1, inimigos, qtd_inimigos_base, torres_menores,
                               qtd_torres_base, 0);
  }

  for (int r = round_inicial; r <= TOTAL_ROUNDS; r++) {
    int res =
        menu_e_executar_round(r, &qtd_inimigos_base, &qtd_torres_base,
                              &inimigos, &torres_menores, torre_principal);

    if (res == 2) {
      liberar_tudo(&inimigos, &torres_menores, &torre_principal);
      return 2;
    }
    if (res == 0 || torre_principal->vida <= 0) {
      printf(
          "\n=== DERROTA! A Torre Principal foi destruida no Round %d. ===\n",
          r);
      liberar_tudo(&inimigos, &torres_menores, &torre_principal);
      return 0;
    }
    printf("\n[OK] Round %d concluido!\n", r);

    if (r < TOTAL_ROUNDS) {
      int qtd_resetada_inimigos = qtd_inimigos_base;
      int qtd_duplicada_inimigos = qtd_resetada_inimigos * 2;
      int qtd_resetada_torres = qtd_torres_base;
      int qtd_nova_torres = qtd_resetada_torres + 2;
      qtd_inimigos_base = qtd_duplicada_inimigos;
      qtd_torres_base = qtd_nova_torres;

      if (!alocar_ou_realocar_arrays(&inimigos, qtd_inimigos_base,
                                     &torres_menores, qtd_torres_base, 0)) {
        liberar_tudo(&inimigos, &torres_menores, &torre_principal);
        return 1;
      }
      configurar_entidades_round(r + 1, inimigos, qtd_inimigos_base,
                                 torres_menores, qtd_torres_base, 1);
      printf("[i] Proximo round: Inimigos = %d (2x %d) | Torres Menores = %d "
             "(%d + 2)\n",
             qtd_inimigos_base, qtd_resetada_inimigos, qtd_torres_base,
             qtd_resetada_torres);
    }
  }

  printf("\n=== VITORIA! Voce defendeu a Torre Principal por todos os %d "
         "rounds! ===\n",
         TOTAL_ROUNDS);
  liberar_tudo(&inimigos, &torres_menores, &torre_principal);
  return 0;
}
