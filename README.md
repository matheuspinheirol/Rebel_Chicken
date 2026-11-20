# _Rebel Chicken_

***Rebel Chicken*** é um jogo programado em C a partir da biblioteca raylib.

Você é uma galinha insatisfeita com sua vida na fazenda. Desvie dos obstáculos para se afastar o máximo possível do cativeiro, mas cuidado: a sua ansiedade te torna cada vez mais rápido e suscetível a colidir e ser capturado.

## Características
- **Fácil de Aprender:** O jogo apresenta apenas um comando simples (`Espaço` para subir).
- **Faixa Etária Abrangente:** _Rebel Chicken_ é recomendado e jogável por jogadores de todas as idades.
- **Dificuldade Progressiva:** A experiência vai se tornando mais difícil com o andar do jogo, mantendo o jogador estimulado e concentrado.
- **Jogabilidade Solo:** _Rebel Chicken_ é jogado sozinho, servindo como passatempo em momentos solitários.

## Software e Bibliotecas Utilizados
- **C:** Linguagem utilizada para a definição de constantes, construção de structs, funções e a lógica geral do jogo.
- **raylib:** Biblioteca usada para a construção da interface jogável e leitura de comandos do usuário.

## Instruções de Instalação
Pré-requisitos: GCC, Make, Git e a biblioteca raylib.

1. Clone o repositório:
   ```bash
   git clone [https://github.com/matheuspinheirol/Rebel_Chicken.git](https://github.com/matheuspinheirol/Rebel_Chicken.git)
   ```

2. Acesse a pasta inserindo no terminal:
   ```bash
   cd rebel-chicken
   ```

3. Instale a biblioteca raylib:
   - **Windows:** instale o [instalador oficial da raylib](https://www.raylib.com/) (inclui GCC e Make) e garanta que o código-fonte da raylib esteja em `C:/raylib/raylib/src`, pois é o caminho usado no `Makefile`. Se a sua instalação estiver em outra pasta, altere os caminhos `-I` e `-L` na regra `all` do `Makefile`.
   - **macOS (Homebrew):**
     ```bash
     brew install raylib
     ```
   - **Linux (Debian/Ubuntu):**
     ```bash
     sudo apt install build-essential git libraylib-dev
     ```
     Se o pacote não estiver disponível na sua distribuição, siga o [guia oficial de compilação da raylib](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux).
   - **Linux (Fedora):**
     ```bash
     sudo dnf install gcc make git raylib-devel mesa-libGL-devel libX11-devel
     ```

## Instruções de Uso

Execute o comando correspondente ao seu sistema, sempre a partir da pasta do projeto. Ele compila e abre o jogo.

| Sistema | Comando |
|---------|---------|
| Windows | `make run` |
| macOS | `make mac` |
| Linux | `make linux` |

No Windows, também é possível compilar com `make` e depois abrir o `rebel_chicken.exe`.

### Controles

| Tecla | Ação |
|-------|------|
| `Espaço` (segurar) | A galinha sobe |
| `Espaço` (soltar) | A galinha desce |

### Objetivo

- Colete o milho para ganhar pontos (`+10` por milho).
- Desvie do arame farpado e dos falcões.
- Sobreviva o máximo que conseguir: a dificuldade aumenta com o tempo.
