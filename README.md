# Tower Defense (C / Linux)

Jogo de *Tower Defense* em modo texto (ASCII) implementado em C para terminal Linux, com menu principal, movimentação de inimigos em animação simples, posicionamento de torres menores, salvamento/carregamento de partida e múltiplos rounds.

## Estrutura do projeto

* `td.h`: arquivo de cabeçalho (*header*) com constantes do jogo (`MAPA_TAM`, `TOTAL_ROUNDS`, `SAVE_FILE`, `DELAY_MS`), definições das `struct`s (`Inimigo`, `TorreMenor`, `TorrePrincipal`) e protótipos das funções.
* `jogo.c`: implementa a lógica principal do jogo, alocação/realocação dinâmica de memória, animação no terminal, sistema de save/load (`save_td.txt`) e execução dos rounds.
* `menu.c`: implementa o menu principal no terminal, verificação/resumo do arquivo de save e execução do binário `./jogo` via `fork`, `execl` e `waitpid`.
* `save_td.txt`: arquivo texto gerado/lido ao salvar ou carregar uma partida em andamento.

## Compilação e execução

Compile os dois arquivos `.c` (ambos incluídos com `td.h`) usando `gcc` com `-Wall -Wextra`:

```bash
gcc -Wall -Wextra jogo.c -o jogo
gcc -Wall -Wextra menu.c -o menu
