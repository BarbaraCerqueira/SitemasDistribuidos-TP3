# Trabalho 3 da disciplina Sistemas Distribuidos, período 2023.1, UFRJ. 
Implementa um algoritmo centralizado de exclusão mútua distribuído.

### client_tests.sh
Arquivo para execução dos testes propostos no anúncio do trabalho.

### cliente.c
Arquivo com o código que gera processos clientes

### compile.sh
Script para compilação dos arquivoc .c

### log_validator.py
Valida a saída do log gerado pelo coordenador

### plot_graphics.gp
Arquivo para plotagem dos gráficos dos resultados produzidos pelo coordenador e pelos clientes

### result_validator.py
Valida a saída no arquivo resultado.txt gerado pelo coordenador

### /csv_data
Pasta com dados gerados com os casos de teste, como tempo de construção dos logs, validação dos logs, entre outros, salvos em arquivos .csv usados como base para os gráficos. Eles estão nomeados como csv_data_r${k}_k${k}.txt. onde r e k são os argumentos dados para o programa cliente em cada teste.

### /graphics
Pasta com os gráficos gerados dos resultados dos testes.

### /outputs
Pasta com os arquivos log.txt e resultado.txt de cada um dos casos de teste. Eles estão nomeados como log_n${n}_r${k}_k${k}.txt e resultado_n${n}_r${k}_k${k}.txt, onde n, r e k são os argumentos dados para o programa cliente em cada teste.

---
### Funcionamento do programa
O programa consite em um arquivo fonte (coordenador.c) que roda o processo coordenador. O coordenador roda em uma thread principal, encarregada de receber pedidos dos clientes, uma thread de interface, por meio da qual o usuário interage com o programa, usando-a para exibir a fila atual de pedidos dos processos clientes e quantas vezes cada processo foi atentido, e as threads usadas para processar os pedidos dos clientes, criadas quando novos pedidos chegam.  
O arquivo cliente.c roda os processos clientes, a ele são passados os argumentos n (número de processos clientes), r (número de repetições) e k (número de segundos a aguardar na região crítica. Ao executar, o arquivo cria n processos com a função fork(), e cada um deles age sobre uma região crítica, dentro da qual escrevem seus PIDs e os seus horários atuais em um arquivo chamado resultado.txt. Eles repetem essa ação r vezes e, após cada vez, aguardam k segundos antes de sair da região crítica. O acesso à região crítica é coordenado com a troca de mensagens com o coordenador. O coordenador registra cada mensagem que recebe em um arquivo log.txt.  

### Como usar
Compile os arquivos .c rodando ./compile.sh`. Para rodar o coordenador, use ./coordenador. Para o cliente, use ./cliente <n> <r> <k> após rodar o coordenador.  
Para rodar os testes do trabalho, use ./client_tests com o coordenador rodando. Os testes podem requerir o programa bc, você pode instalá-lo no Linux com sudo apt install bc. Após rodar os testes, os arquivos nas pastas graphics, outputs e csv_data serão refeitos.


