# Intercepto

Intercepto é uma ferramenta em **C++** projetada para monitorar requisições HTTP em tempo real. Ele atua como um **proxy** simples, interceptando e exibindo detalhes como headers, método, URL e outros dados relevantes das requisições.

## **Características**

- Captura de requisições HTTP.
- Exibição de detalhes como:
  - Método HTTP
  - URL
  - Headers
- Arquitetura modular para fácil extensão.

## **Requisitos**

- Compilador C++ com suporte a C++17.
- Boost (`boost` e `boost-libs`).
- CMake (mínimo 3.10).

  ## **Novidades**
  - Irei disponibilizar uma interface criada usando qt para conseguir fazer análise de dados com os payloads.

## **Instalação**

1. Clone o repositório:
   ```bash
   git clone https://github.com/debjordan/intercepto.git
   cd intercepto
