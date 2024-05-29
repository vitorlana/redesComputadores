#!/bin/zsh

# Navegar para a pasta cod e compilar os programas
cd ../cod
make

# Navegar de volta para a pasta scripts
cd ../scripts

# Executar o servidor em terminais separados
osascript -e 'tell application "Terminal" to do script "cd '$(pwd)'/../cod; ./server v6 12345; exec zsh"' &
osascript -e 'tell application "Terminal" to do script "cd '$(pwd)'/../cod; ./server v6 54321; exec zsh"' &

# Aguardar para garantir que os servidores estejam prontos
sleep 5

# Executar cada cliente em terminais separados usando redirecionamento de input e mantendo o terminal aberto
osascript -e 'tell application "Terminal" to do script "cd '$(pwd)'; ./../cod/client ::1 12345 54321; exec zsh"' &
osascript -e 'tell application "Terminal" to do script "cd '$(pwd)'; ./../cod/client ::1 12345 54321; exec zsh"' &



