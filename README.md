Arquitetura Atual do Projeto
A arquitetura do sistema foi projetada para interceptar e monitorar pacotes de rede em tempo real em ambientes Linux. O sistema é dividido em várias camadas que se integram para realizar a captura, análise e manipulação dos pacotes HTTP ou TCP/UDP. Vamos detalhar cada componente da arquitetura, mostrando as interações entre eles.

1. Camada de Captura de Pacotes (libpcap)
Descrição:
A primeira camada é responsável pela captura de pacotes de rede que passam pela máquina em modo promíscuo. Isso inclui pacotes TCP/UDP provenientes de dispositivos na rede local e outras VLANs acessíveis.

Tecnologias Utilizadas:
libpcap (para Linux): Biblioteca utilizada para capturar pacotes da rede, incluindo pacotes de outras máquinas ou VLANs.
Fluxo:
A interface de rede da máquina é configurada para modo promíscuo usando a libpcap.
Pacotes de rede são capturados em tempo real, independentemente de estarem destinados à máquina ou não.
Filtros BPF (Berkeley Packet Filter) são aplicados para capturar pacotes de interesse (por exemplo, pacotes TCP nas portas 80, 5000, 1880, etc.).
Responsabilidades:
Interceptação de pacotes de rede em tempo real.
Aplicação de filtros para capturar pacotes de interesse.
Habilitação do modo promíscuo para capturar pacotes de outras VLANs.
2. Camada de Processamento de Pacotes (Proxy HTTP)
Descrição:
Após a captura dos pacotes, a segunda camada processa as requisições HTTP, permitindo visualizar detalhes como métodos HTTP, cabeçalhos e corpo das requisições/respostas. Essa camada pode também realizar a modificação ou redirecionamento de pacotes conforme necessário.

Tecnologias Utilizadas:
C++ com Qt (para interface gráfica): Para o desenvolvimento do código de captura e análise.
Boost.Asio (para manipulação de conexões de rede): Para criar um servidor proxy HTTP que interage com os pacotes capturados.
Fluxo:
O pacote capturado é analisado para identificar requisições HTTP (por exemplo, cabeçalhos HTTP, URL e corpo da requisição).
O proxy manipula a requisição, enviando-a para o servidor de destino e capturando a resposta.
A resposta do servidor é analisada e enviada de volta para o cliente que fez a requisição.
Responsabilidades:
Processar pacotes HTTP capturados.
Exibir informações sobre as requisições e respostas HTTP.
Modificar ou redirecionar pacotes conforme necessário.
Gerenciar a comunicação com o servidor de destino.
3. Camada de Interface Gráfica (Qt)
Descrição:
A interface gráfica será responsável pela interação com o usuário. O usuário poderá configurar quais pacotes monitorar (por exemplo, selecionar as portas TCP para monitorar) e visualizar os pacotes interceptados em tempo real.

Tecnologias Utilizadas:
Qt Framework: Para o desenvolvimento da interface gráfica interativa.
Fluxo:
O usuário define as configurações de monitoramento, como portas TCP ou interfaces de rede a serem monitoradas.
As configurações são passadas para o módulo de captura de pacotes.
A interface exibe, em tempo real, as requisições capturadas, com a possibilidade de filtrar por IPs, tipos de pacotes, etc.
Responsabilidades:
Exibir as requisições HTTP em tempo real.
Permitir a configuração das opções de monitoramento.
Oferecer uma interface para visualizar detalhes das requisições (IP, método HTTP, cabeçalhos, corpo da requisição, etc.).
Fornecer controles para o usuário, como filtros e flags para controlar o monitoramento.
4. Camada de Configuração e Controle
Descrição:
Esta camada permite ao usuário definir as configurações do sistema, como selecionar quais portas e interfaces de rede monitorar, além de fornecer controles adicionais (como autenticação por IP ou senha).

Fluxo:
O usuário configura as opções de monitoramento, como portas e interfaces.
As configurações são salvas e aplicadas à camada de captura de pacotes e à interface gráfica.
O sistema executa a captura conforme as configurações definidas pelo usuário, e a interface gráfica exibe as informações de forma interativa.
Arquitetura de Comunicação:
A arquitetura segue um fluxo em que a camada de captura de pacotes interage diretamente com a camada de proxy HTTP. O proxy, por sua vez, interage com a interface gráfica para fornecer feedback ao usuário.

Captura de pacotes (libpcap):

Pacotes são capturados pela interface de rede em modo promíscuo.
Filtros de pacotes (BPF) são aplicados para capturar apenas os pacotes relevantes (por exemplo, pacotes nas portas 80, 5000, 1880).
Processamento de pacotes (Proxy HTTP):

O proxy HTTP processa os pacotes HTTP capturados, obtendo informações como métodos, cabeçalhos e corpo da requisição.
A comunicação com o servidor de destino é realizada, e a resposta do servidor é capturada e redirecionada para o cliente.
Interface Gráfica (Qt):

A interface gráfica exibe informações sobre os pacotes HTTP interceptados e permite ao usuário configurar o monitoramento de pacotes (seleção de interfaces de rede e portas TCP a serem monitoradas).
Configurações e Controles:

O usuário pode definir configurações de monitoramento, como a escolha das interfaces de rede e as portas de comunicação.
A interface permite configurar flags e filtros para refinar a captura e visualização de pacotes.
Diagrama Simplificado da Arquitetura
lua

+------------------+     +---------------------+      +-----------------+
|  Camada de       |     |  Camada de          |      |  Camada de      |
|  Captura de      |---->|  Processamento      |<---->|  Interface      |
|  Pacotes (libpcap)|     |  de Pacotes (Proxy) |      |  Gráfica (Qt)   |
|                  |     |                     |      |                 |
+------------------+     +---------------------+      +-----------------+
         ^                        ^
         |                        |
         v                        v
 +----------------------+
 | Camada de Configuração|
 | e Controle            |
 +----------------------+
Recursos de Monitoramento e Controle
Seleção de Interfaces de Rede:

O usuário pode escolher as interfaces de rede para monitorar, incluindo interfaces físicas (Ethernet, Wi-Fi) e virtuais (VLANs).
Filtragem de Portas TCP:

A interface gráfica permite selecionar as portas TCP a serem monitoradas (80, 5000, 1880, etc.).
Visualização em Tempo Real:

As requisições HTTP são exibidas em tempo real na interface gráfica, com informações detalhadas (cabeçalhos, corpo da requisição, status da resposta).
Controle de Flags:

O usuário pode marcar flags para incluir ou excluir pacotes de determinados IPs, configurações de autenticação (usuário/senha), ou especificar configurações avançadas de filtragem.
Conclusão
Essa arquitetura modular permite uma abordagem flexível e escalável para monitoramento de pacotes de rede. Com a captura de pacotes em modo promíscuo via libpcap, processamento avançado no proxy HTTP, e uma interface gráfica interativa, o sistema oferece uma solução robusta e configurável para interceptação e análise de tráfego de rede em tempo real, com ênfase no controle do usuário e visualização clara das informações.


intercepto/
├── CMakeLists.txt                # Arquivo de configuração do CMake
├── README.md                     # Documentação geral do projeto
├── .gitignore                    # Arquivo para ignorar arquivos temporários do git

├── include/                      # Diretório contendo os cabeçalhos de classes e bibliotecas
│   ├── proxy.h                   # Definição da classe Proxy (responsável por processar as requisições HTTP)
│   ├── capture.h                 # Definição da classe Capture (responsável pela captura de pacotes)
│   └── config.h                  # Definição da classe Config (responsável pelas configurações do sistema)

├── src/                          # Diretório contendo os arquivos fontes do projeto
│   ├── main.cpp                  # Função principal do programa, com interação com o usuário
│   ├── proxy.cpp                 # Implementação da classe Proxy (processamento de pacotes HTTP)
│   ├── capture.cpp               # Implementação da classe Capture (captura de pacotes com libpcap)
│   └── config.cpp                # Implementação da classe Config (leitura e aplicação das configurações do usuário)

└── resources/                    # Recursos auxiliares como arquivos de configuração, templates, etc.
    ├── config.json               # Arquivo de configuração do projeto, armazenando as opções do usuário
    └── icons/                    # Diretório com ícones e imagens utilizadas pela interface gráfica
        └── logo.png              # Exemplo de imagem/logo para a interface gráfica
