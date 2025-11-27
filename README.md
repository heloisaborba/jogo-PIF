# jogo-PIF

# Tower Defense

## 📖 Descrição

Jogo tower defense com temática medieval, onde o jogador deve proteger seu reino contra hordas de inimigos como goblins, orcs e cavaleiros que avançam por caminhos em direção ao castelo.
O jogador precisa posicionar torres de defesa e heróis estrategicamente para impedir que os inimigos alcancem o portão.
O objetivo é defender o castelo e gerenciar os recursos disponíveis, garantindo que as defesas sejam fortes o suficiente para resistir a cada onda de ataques.

---

## 🧰 Tecnologias e Ferramentas

- C: Linguagem usada na programação das mecânicas do jogo, incluindo IA dos inimigos, controle das torres e lógica de colisões.

---

## 🏰 Elementos Principais

- Torres de Defesa: É o pronto principal do jogo, pois o objetivo dos vilões é atacar a torre ate deixa-lá sem vida.

- Heróis: Personagens especiais que o jogador pode posicionar estrategicamente para reforçar defesas em áreas críticas.

- Inimigos: Vão vir hordas de inimigos, cada um com velocidade, resistência e comportamento distintos.

- Recursos: Moedas obtidas ao derrotar inimigos, usadas para construir ou aprimorar torres e heróis.

---

## 🛡️ Personagens

### Herois:
- Guerreiro
- Mago
- Paladino
- Bardo

### Vilões:
- Goblim
- Spectro
- Necromante
- Dragão

---

## 💻 Instrução de compilação
É necessario ter o wsl (windows subsystem for linux) instalado em seu
computador. <br>
Segue o guia de compilação: <br>

Verificar se os pré requisitos estão instalados em seu ambiente wsl (como compilador c/c++,
ray.lib e cmake.

• Clone o projeto do github em sua máquina <br>
• Abra o prompt de comando (cmd), e insira o comando: wsl install; <br>
• Iicie o wsl, digite wsl e aperte enter no terminal e execute o comando:
sudo apt-get install make; <br>
• Para instalar o raylib, primeiro atualizar o sistema: sudo apt update &&
sudo apt upgrade -y <br>
• Instale as dependencias necessárias para raylib: sudo apt install git
build-essential cmake libasound2-dev libpulse-dev libx11-dev libxcursordev libxinerama-dev libxi-dev libxrandrdev libgl1-mesa-dev libudev-dev <br>

• Clone o repositório em sua máquina: git clone
https://github.com/raysan5/raylib.git; e em seguida insira: cd
raylib; <br>
• Codigo para compilar e instalar: <br>
    mkdir build <br>
    cd build <br>
    cmake .. <br>
    make -j$(nproc) <br>
    sudo make install <br>

Depois de instalar todas as dependências, para rodar o código basta abrir o arquivo: cd ("caminho da pasta e nome do projeto clonado") <br>
Ao fazer isso, escreva no WSL “./tower_defense” e aproveite o jogo!

## video do jogo rodando
[Assista ao Trailer do Jogo Aqui!](https://youtu.be/YhF9yTNmfJI?feature=shared)





