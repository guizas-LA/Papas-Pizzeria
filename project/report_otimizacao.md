# Relatório de Otimização 

 

---

## 1. Contexto

Durante o desenvolvimento do projeto, verificou-se que o jogo apresentava um **atraso visual significativo** numa das máquinas da equipa, enquanto nas restantes corria de forma fluida. O jogo está configurado a **60 FPS**, o que significa que o motor de renderização tem de completar um frame inteiro em menos de **16,7 ms** por iteração do ciclo principal.

A análise ao código revelou um bottleneck crítico na função de desenho de círculos, que é usada intensivamente em cada frame durante o estado de preparação da pizza.

---

## 2. Diagnóstico

### 2.1 Função original — `draw_circle`

A implementação original percorria todos os píxeis dentro de um quadrado de lado `2r`, verificando individualmente se cada ponto pertencia ao círculo, e chamava `vg_draw_pixel` para cada um:

```c
void draw_circle(int cx, int cy, int radius, uint32_t color) {
  int y, x;
  for (y = -radius; y <= radius; y++) {
    for (x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius) {
        vg_draw_pixel(cx + x, cy + y, color);
      }
    }
  }
}
```


### 2.2 Impacto por frame

Cada frame do estado `PLAYING_PREPARE_PIZZA` chamava `draw_circle` múltiplas vezes


A 60 FPS, isto equivale a cerca de **12 milhões de chamadas `vg_draw_pixel` por segundo**, apenas para a renderização da pizza.

### 2.3 Por que era diferente entre máquinas

A diferença de desempenho entre as máquinas da equipa deve-se à forma como o MINIX/QEMU virtualiza o acesso à memória de vídeo. Em hardware mais lento ou com configuração de VM diferente, cada chamada `vg_draw_pixel` tem latência superior, amplificando o problema.

---

## 3. Solução

### 3.1 Algoritmo de scanlines com `vg_draw_rectangle`

Em vez de pintar píxel a píxel, a solução percorre cada linha horizontal do círculo e desenha toda a linha de uma só vez com uma única chamada `vg_draw_rectangle`. A largura de cada scanline é calculada com raiz quadrada inteira.

```c
static int isqrt(int n) {
  int x, y;
  if (n <= 0) return 0;
  x = n; y = (x + 1) / 2;
  while (y < x) { x = y; y = (x + n / x) / 2; }
  return x;
}

void draw_circle(int cx, int cy, int radius, uint32_t color) {
  int y, hw, r2 = radius * radius;
  for (y = -radius; y <= radius; y++) {
    hw = isqrt(r2 - y * y);
    vg_draw_rectangle(cx - hw, cy + y, 2 * hw + 1, 1, color);
  }
}
```



---

## 4. Resultado

Após a correção, o jogo passou a correr a 60 FPS de forma estável em todas as máquinas da equipa, incluindo a que apresentava o atraso. A fluidez da animação da pizza (incluindo o efeito de escurecimento progressivo no forno) ficou uniforme independentemente do número de toppings colocados.

---

## 5. Conclusão

O problema não estava na lógica do jogo nem na frequência de atualização, mas na **escolha de primitiva gráfica**: usar `vg_draw_pixel` para preencher áreas em vez de `vg_draw_rectangle`. Em modo gráfico direto sobre framebuffer, o overhead de cada chamada individual acumula-se rapidamente.

A regra prática: **preencher regiões com retângulos, não com píxeis**. Um círculo de raio r tem `πr²` píxeis mas apenas `2r+1` scanlines — essa diferença é a origem de toda a ganho de desempenho obtido.
