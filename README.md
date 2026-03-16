# Intercepto

Intercepto é uma ferramenta de interceptação de tráfego HTTP escrita em **C++17**. Funciona como um **proxy transparente** — redireciona requisições com base no header `Host`, exibindo em tempo real método, URL, headers, payload e latência de cada ciclo requisição/resposta.

Disponível em dois modos: **CLI** (terminal colorido) e **GUI desktop** (interface Qt).

---

## Funcionalidades

- Proxy HTTP transparente (forward automático via header `Host`)
- Captura e exibição de:
  - Método HTTP (GET, POST, PUT, DELETE, PATCH...)
  - URL e host de destino
  - Headers completos de requisição e resposta
  - Payload (body) com truncamento inteligente
  - Latência por requisição (ms)
- Thread pool configurável (padrão: `hardware_concurrency`)
- Monitoramento de múltiplas portas simultâneas (modo GUI)
- Estatísticas ao encerrar: total de requisições, bytes, latência média, distribuição de status codes
- Saída colorida no terminal (ANSI) com timestamps de milissegundos
- Interface desktop com tema escuro (Catppuccin Mocha)
- Pacote `.deb` para instalação via `dpkg`

---

## Requisitos

| Dependência | Versão mínima |
|---|---|
| Compilador C++ | GCC 10+ / Clang 11+ (C++17) |
| CMake | 3.16+ |
| Boost | 1.74+ (`libboost-system-dev`) |
| Qt *(apenas GUI)* | 6.4+ (`qt6-base-dev`) |

**Debian/Ubuntu:**
```bash
sudo apt install libboost-dev libboost-system-dev cmake build-essential
# Opcional (GUI):
sudo apt install qt6-base-dev
```

---

## Compilação

```bash
git clone https://github.com/debjordan/intercepto.git
cd intercepto
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Isso gera dois binários em `build/`:
- `intercepto` — modo CLI
- `intercepto-gui` — interface desktop (gerado apenas se Qt6 for encontrado)

---

## Uso

### Modo CLI

```bash
./intercepto <host> <porta>
```

**Exemplos:**

```bash
# Escuta em todas as interfaces na porta 8080
./intercepto 0.0.0.0 8080

# Escuta apenas em loopback
./intercepto 127.0.0.1 8080
```

Configura o proxy no cliente (ex.: curl, browser, Postman):

```bash
curl -x http://127.0.0.1:8080 http://qualquer-site.com/api/endpoint
```

Pressione `Ctrl+C` para encerrar e exibir as estatísticas finais.

**Exemplo de saída:**

```
[08:22:31.090] #0001 -> REQUISICAO ----------------------------
  Metodo  GET  ->  example.com:80
  URL     http://example.com/
  Headers Host: example.com
          User-Agent: curl/7.88.1
  Payload [vazio]

[08:22:31.136] #0001 <- RESPOSTA (45ms) --------------------
  Status  200 OK
  Headers Content-Type: text/html
  Payload <!doctype html>...

==========================================
         Estatisticas Finais
==========================================
  Porta         : 8080
  Tempo ativo   : 12s
  Requisicoes   : 5
  Latencia media: 62 ms
  Status codes:
    200  ->  4 req(s)
    404  ->  1 req(s)
```

### Modo GUI

```bash
./intercepto-gui
```

1. Digite o número da porta no campo **Porta** e clique em **Iniciar**
2. Repita para adicionar quantas portas quiser monitorar simultaneamente
3. As requisições aparecem em tempo real na tabela central
4. Clique em qualquer linha para ver os headers e body completos no painel inferior
5. Use **Parar Porta** para encerrar o proxy de uma porta específica
6. Use **Limpar Log** para limpar a tabela sem parar os proxies

---

## Instalação via .deb

```bash
cd build
cpack                         # gera intercepto-1.0.0-Linux.deb
sudo dpkg -i intercepto-1.0.0-Linux.deb

# Uso após instalação:
intercepto 0.0.0.0 8080
intercepto-gui
```

---

## Arquitetura

```
include/
  proxy_event.h       # Struct ProxyEvent — dados puros de um ciclo req/resp
  proxy.h             # Classe Proxy — thread pool + acceptor + stop() real
  proxy_bridge.h      # QObject bridge — cruza eventos Boost -> thread Qt
  proxy_manager.h     # Gerencia múltiplas instâncias de Proxy por porta
  mainwindow.h        # Janela principal Qt
  logger.h            # Cores ANSI, timestamps, helpers de formatação

src/
  main.cpp            # Entry point CLI
  gui_main.cpp        # Entry point GUI (QApplication)
  proxy.cpp           # Loop de aceitação, forward HTTP, emissão de eventos
  proxy_bridge.cpp    # Post thread-safe via QMetaObject::invokeMethod
  proxy_manager.cpp   # Ciclo de vida dos proxies por porta
  mainwindow.cpp      # Layout Qt, tabela de requisições, painel de detalhes
```

**Fluxo de dados (modo GUI):**

```
Thread pool Boost  -->  ProxyEvent  -->  ProxyBridge::post()
                                              |
                                    QueuedConnection (thread-safe)
                                              |
                                    ProxyBridge::requestCaptured  (signal)
                                              |
                                    MainWindow::onRequestCaptured  (slot Qt)
                                              |
                                         Tabela + Detalhes
```

---

## Próximos passos

- [ ] **Filtros na tabela** — filtrar por porta, método, status code ou host
- [ ] **Exportação** — salvar requisições capturadas em JSON ou HAR
- [ ] **Suporte a HTTPS** — interceptação via MITM com certificado auto-assinado (Boost.ASIO SSL)
- [ ] **Edição de requisições** — modificar headers/body antes do forward (modo intercept)
- [ ] **Replay** — reenviar uma requisição capturada com um clique
- [ ] **Busca no log** — busca por texto nos headers e body
- [ ] **Ícone e `.desktop`** — integração completa com o sistema de aplicativos do Linux
- [ ] **Suporte a WebSocket** — interceptação de frames WS

---

## Licença

MIT
