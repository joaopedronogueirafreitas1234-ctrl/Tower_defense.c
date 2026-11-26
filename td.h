#ifndef TD_H
#define TD_H

#define MAPA_TAM 20
#define NUM_LINHAS 3
#define TOTAL_ROUNDS 3
#define SAVE_FILE "save_td.txt"
#define DELAY_MS 350

typedef struct {
  int vida;
  int posicao;
  int linha;
  int qtd;
} Inimigo;

typedef struct {
  int dano;
  int posicao;
  int linha;
  int alcance;
  int quantidade;
} TorreMenor;

typedef struct {
  int vida;
} TorrePrincipal;

/* Funcoes do menu (menu.c) */
int verificar_save(void);
void mostrar_resumo_save(void);
int executar_jogo(int carregar_save);

/* Funcoes auxiliares e do jogo (jogo.c) */
void dormir_ms(int ms);
void limpar_tela_animacao(void);
void limpar_buffer(void);
int ler_inteiro(const char *prompt, int min_val, int max_val);

void liberar_tudo(Inimigo **inimigos, TorreMenor **torres_menores,
                  TorrePrincipal **torre_principal);
int alocar_ou_realocar_arrays(Inimigo **inimigos, int qtd_inimigos,
                              TorreMenor **torres_menores, int qtd_torres,
                              int eh_primeiro_round);
void configurar_entidades_round(int round, Inimigo *inimigos, int qtd_inimigos,
                                TorreMenor *torres_menores, int qtd_torres,
                                int preservar_posicoes_torres);

int salvar_jogo(const char *caminho, int round_atual, int qtd_inimigos_base,
                int qtd_torres_base, const Inimigo *inimigos,
                const TorreMenor *torres_menores,
                const TorrePrincipal *torre_principal);
int carregar_jogo(const char *caminho, int *round_atual, int *qtd_inimigos_base,
                  int *qtd_torres_base, Inimigo **inimigos,
                  TorreMenor **torres_menores, TorrePrincipal *torre_principal);

void imprimir_mapa_frame(const Inimigo *inimigos, int qtd_inimigos,
                         const TorreMenor *torres_menores, int qtd_torres,
                         const TorrePrincipal *torre_principal,
                         const int tiros_na_pos[NUM_LINHAS][MAPA_TAM],
                         int passo);
int menu_e_executar_round(int round, int *qtd_inimigos_base,
                          int *qtd_torres_base, Inimigo **inimigos,
                          TorreMenor **torres_menores,
                          TorrePrincipal *torre_principal);

#endif
