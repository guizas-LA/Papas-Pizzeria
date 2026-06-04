# Otimização Gráfica

## Introdução

O projeto Papa's Pizzeria é um remake do jogo clássico, desenvolvido em C para o ambiente LCOM/Minix, no âmbito da unidade curricular de Laboratório de Computadores (LCOM). A aplicação segue uma arquitetura Model-View-Controller (MVC) e utiliza dispositivos de I/O como timer, teclado, rato, placa gráfica e RTC.

Durante o desenvolvimento, foi identificado um problema significativo de desempenho gráfico que comprometia a jogabilidade. Este documento descreve o problema, a análise realizada e a solução implementada.

## Problema Identificado

Ao executar o jogo no ambiente Minix, verificou-se que a aplicação apresentava um nível de lag extremamente elevado. O rato reagia com vários segundos de atraso, o relógio (RTC) praticamente não avançava entre frames e a interação com os menus era quase impossível. O jogo tornava-se, na prática, inutilizável.

A causa não residia na lógica do jogo nem na gestão de interrupções, mas sim na camada de renderização gráfica — especificamente, nas funções da biblioteca gráfica desenvolvida no lab5.

## Análise da Solução Inicial

A biblioteca gráfica do lab5 disponibiliza funções como `vg_draw_pixel`, `vg_draw_hline`, `vg_draw_rectangle`, `vg_draw_xpm`, `vg_clear_buffer` e `vg_swap_buffer`. Estas funções operam sobre um *hidden buffer* (double buffering), o que é conceptualmente correto. No entanto, a implementação interna destas funções apresentava ineficiências severas:

- **`vg_draw_pixel`**: Cada chamada realiza uma verificação de limites, calcula o offset no buffer e executa um `memcpy` de apenas 3 bytes (modo 24-bit). Embora individualmente rápida, o custo acumula-se quando invocada centenas de milhares de vezes por frame.

- **`vg_draw_hline`**: Implementada como um ciclo que chama `vg_draw_pixel` para cada pixel da linha, duplicando verificações de limites e cálculos de offset em cada iteração.

- **`vg_draw_rectangle`**: Implementada como um ciclo que chama `vg_draw_hline` para cada linha do retângulo. Para um retângulo de dimensão `w × h`, o número total de chamadas a `vg_draw_pixel` é `w × h`.

- **`vg_draw_xpm`**: Percorre a imagem XPM pixel a pixel, copiando cada pixel individualmente com `memcpy` de 3 bytes. Para o fundo do ecrã (800×600 pixels), isto resulta em 480.000 operações `memcpy` individuais por frame.

- **`vg_clear_buffer`**: Percorre todo o frame buffer e copia a cor pixel a pixel, resultando igualmente em 480.000 operações por frame.

- **`vg_swap_buffer`**: Implementada como um único `memcpy` de todo o buffer, sendo a única operação eficiente da biblioteca.

No contexto do jogo, um único frame no menu principal envolvia:
- 480.000 chamadas para limpar o ecrã (`vg_clear_buffer`)
- 480.000 chamadas para desenhar o fundo XPM (`vg_draw_xpm`)
- Milhares de chamadas adicionais para botões, texto e cursor

O total ultrapassava facilmente **1 milhão de chamadas a funções por frame**, tornando impossível manter uma taxa de atualização de 60 FPS.

## Solução Implementada

Uma vez que as bibliotecas dos labs não podem ser alteradas, foi criado um módulo independente composto por dois ficheiros:

- **`include/fast_draw.h`** — Interface pública com as declarações das funções.
- **`src/view/fast_draw.c`** — Implementação das funções otimizadas.

### Princípios da Otimização

A estratégia central consiste em **aceder diretamente ao frame buffer** sem passar pelas funções intermédias do lab5, eliminando o overhead de chamadas a funções e verificações de limites redundantes. As principais técnicas utilizadas são:

1. **Mapeamento próprio da VRAM**: O módulo `fast_draw` mapeia a memória de vídeo de forma independente durante a inicialização (`fast_draw_init`), obtendo um ponteiro direto para a VRAM e alocando o seu próprio back-buffer.

2. **Operações por linha em vez de por pixel**: Em vez de copiar 3 bytes de cada vez, as operações utilizam `memcpy` sobre linhas inteiras ou blocos contíguos de memória.

3. **Replicação de scanlines**: Para retângulos e limpeza de ecrã, preenche-se uma única linha e replica-se para as restantes através de `memcpy`, evitando recalcular cada pixel.

4. **Clipping integrado**: Todas as funções realizam uma única verificação de limites no início, em vez de verificar cada pixel individualmente.

### Funções Implementadas

| Função | Descrição |
|--------|-----------|
| `fast_draw_init` | Inicializa o módulo, mapeia a VRAM e aloca o back-buffer |
| `fast_clear` | Preenche o ecrã com uma cor sólida |
| `fast_rect` | Desenha um retângulo preenchido com clipping |
| `fast_circle` | Desenha um círculo preenchido por scanlines |
| `fast_pixel` | Escreve um pixel individual com acesso direto ao buffer |
| `fast_xpm` | Copia uma imagem XPM para o buffer por linhas |
| `fast_swap` | Copia o back-buffer para a VRAM |

## Comparação Antes e Depois

A tabela seguinte resume a diferença no número de operações de memória por frame para cada tipo de operação, considerando a resolução 800×600 em modo 24-bit (3 bytes por pixel):

| Operação | Lab5 (antes) | Fast Draw (depois) | Redução |
|----------|-------------|-------------------|---------|
| Desenho de XPM (800×600) | 480.000 `memcpy` de 3 bytes | 600 `memcpy` de 2.400 bytes | ~800× menos chamadas |
| Limpeza do ecrã | 480.000 `memcpy` de 3 bytes | 800 `memcpy` (1 linha) + 599 `memcpy` de 2.400 bytes | ~340× menos chamadas |
| Retângulo (ex: 230×70) | 16.100 `memcpy` de 3 bytes | 230 `memcpy` (1 linha) + 69 `memcpy` de 690 bytes | ~54× menos chamadas |
| Buffer swap | 1 `memcpy` de ~1,44 MB | 1 `memcpy` de ~1,44 MB | Equivalente |

## Impacto no Desempenho

Embora não tenham sido realizados benchmarks formais no ambiente Minix, as melhorias são diretamente observáveis durante a execução do jogo:

- **Eliminação do lag**: O rato responde de forma imediata aos movimentos do utilizador, sem os atrasos de vários segundos verificados anteriormente.

- **Fluidez na renderização**: As transições entre ecrãs (menu, preparação, forno, corte, entrega) ocorrem de forma suave, sem bloqueios percetíveis.

- **Menor carga de processamento**: A redução drástica no número de chamadas a funções e operações de memória liberta tempo de CPU para o processamento de interrupções (timer, teclado, rato), garantindo que os eventos de input são tratados atempadamente.

- **Relógio funcional**: O RTC é atualizado a cada segundo como esperado, uma vez que o ciclo de renderização já não consome a quase totalidade do tempo disponível entre frames.

- **Experiência de jogo viável**: O jogo torna-se efetivamente jogável, permitindo completar pedidos e interagir com todos os elementos da interface.

## Integração com a Arquitetura MVC

A solução implementada respeita integralmente a arquitetura MVC do projeto:

- **Model** (`src/model/`): Não sofreu qualquer alteração. A lógica do jogo, gestão de pedidos e processamento de input permanecem inalterados.

- **View** (`src/view/`): Os ficheiros de renderização (`draw.c`, `draw_utils.c`, `draw_elements.c`, `sprites.c`) foram atualizados para utilizar as funções `fast_*` em vez das funções `vg_*` do lab5. O módulo `fast_draw.c` foi adicionado a esta camada.

- **Game** (`src/game/`): Apenas `loop.c` foi alterado para invocar `fast_draw_init` na inicialização, após o setup da placa gráfica.

- **Labs**: Nenhuma biblioteca de lab foi modificada. As funções do lab5 continuam disponíveis e a biblioteca `libgraphics.a` continua a ser linkada (necessária para `vg_exit`, `set_graphics_mode` e `map_video_memory`).

O novo módulo funciona como uma camada de abstração adicional entre a view e o hardware gráfico, mantendo a separação de responsabilidades da arquitetura original.

## Conclusão

A otimização gráfica realizada através do módulo Fast Draw demonstra como o acesso direto à memória de vídeo e a utilização de operações de memória por bloco podem produzir melhorias significativas de desempenho em sistemas com restrições de hardware como o Minix. A solução foi implementada de forma modular, sem alterar as bibliotecas existentes dos labs e respeitando a arquitetura MVC do projeto, resultando numa experiência de jogo funcional e responsiva.
