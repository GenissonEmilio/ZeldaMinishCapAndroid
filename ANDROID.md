# Minish Cap para Android

Aplicativo independente para Android 9 ou mais recente, **ARM64**. O jogo é
executado pelo gbarecomp a partir de corpora C++ separados para USA e Europe.
A interface Android, o importador e os controles touch são deste fork.

## Instalação e uso

1. Instale o APK ARM64 da seção Releases deste repositório.
2. Toque em **Importar ROM** e escolha sua cópia `.gba` ou um `.zip` contendo
   uma única ROM USA ou Europe original, sem tradução ou patches.
3. Após a confirmação, toque em **Jogar**. Uma cópia verificada fica no
   armazenamento privado do aplicativo; o ZIP original pode ser removido.
   Na ROM Europe (`BZMP`), escolha **Español** nas opções de idioma do jogo.
   Também estão disponíveis English, Français, Deutsch e Italiano. A região
   da última importação é selecionada automaticamente; reimportar a outra
   região troca a seleção sem apagar o seu progresso anterior.
4. Direcional: movimento; A/B e L/R: botões do GBA; START/SELECT: os botões
   correspondentes. Deslizar no direcional permite diagonais, e os botões
   aceitam múltiplos toques ao mesmo tempo.
5. **MENU**, Voltar do Android ou um toque com três dedos abre o menu do
   runtime, com pausa, estados rápidos e ajustes de apresentação/áudio.
   **System → Voltar para o inicio** encerra a partida e retorna à biblioteca.
6. **Controles**, na tela inicial, ajusta tamanho e opacidade para a próxima
   partida. O runtime também aceita controles físicos via SDL2; a validação
   de controle físico depende de um dispositivo compatível conectado.
   A mesma tela permite ligar ou silenciar o som para a próxima partida.
7. **Imagem** oferece **Suave (Scale2x)**, ativado por padrão, e **Pixel
   perfeito**. Suave arredonda contornos escolhendo cores da própria arte,
   sem o borrado de interpolação bilinear. Pixel perfeito mantém os pixels
   originais em blocos uniformes. Durante a partida, troque em **MENU →
   Graphics → Estilo dos pixels**, com efeito imediato e escolha salva.
8. Na ROM USA, abra **START → Itens → R → Álbum de Hyrule**. A nova página fica entre
   Itens e Status, na mesma sequência L/R do inventário, com arte em 240×160.
   O direcional seleciona **Camera** ou uma das seis miniaturas; **A** abre a
   câmera ou a imagem, e **B** retorna. Nas bordas de uma linha, esquerda e
   direita passam às outras páginas de fotos. **L/R** troca de aba do jogo;
   **START** fecha o inventário, inclusive na câmera e na visualização.
9. Na câmera, **A** fotografa a última cena completa antes de abrir o
   inventário. **B** retorna ao álbum. A imagem salva não contém moldura,
   inventário ou controles touch. PNG e miniaturas são processados fora da
   thread do jogo; a câmera mostra a confirmação **LEMBRANCA GUARDADA!**.
   O inventário continua respeitando os bloqueios normais da história.

O álbum do jogo usa os mesmos arquivos da galeria acessível pela tela inicial
ou por **MENU → Album → Gerenciar fotos e anotacoes**. Essa galeria Android
mostra título, data e anotações. Use dois dedos para ampliar, arraste para explorar ou dê dois
toques para ampliar/restaurar. **Anotar** nomeia o local e registra o que
revisitar; **Exportar PNG** usa o seletor Android para guardar uma cópia;
**Excluir** pede confirmação. Até 200 capturas ficam no diretório privado
`captures/`, separado do save. Nenhuma captura antiga é apagada automaticamente.
Atualizações assinadas conservam imagens e notas. Para guardar imagens fora
dos dados do aplicativo, exporte os PNGs antes de desinstalar.

Os controles usam ícones vetoriais originais de espada, escudo, pena, gorro,
mapa, bolsa e brasão. As letras A/B/L/R continuam visíveis; os ícones são
decorativos e não mudam os itens equipados ou as funções originais dos botões.

A página é uma extensão C++ do menu de pausa: um hook no subtask original
insere Álbum entre Itens e Status e assume seus comandos enquanto a página
está visível. A câmera e a visualização usam o renderer do jogo, sem trocar
de Activity Android. O framebuffer original e a ROM permanecem intactos.
Offsets e navegação são restritos à USA verificada; na Europe, o inventário
original permanece sem essa página. A galeria Android continua acessível
pela tela inicial e pelo MENU. A visão ampliada também permanece exclusiva
da USA. A configuração de hooks
fica em `symbols/minishcap_reviewed.toml`. O código gerado é regenerado.

Na versão 0.3.1, a página mantém o fundo e a moldura originais apresentados
pelo jogo, incluindo abas L/R e indicadores A/B. Apenas o título, o conteúdo
interno e a descrição são compostos pelo álbum. As letras são decodificadas
da fonte da ROM USA carregada, sem distribuir uma cópia desses gráficos.
A troca usa uma transição de oito quadros de guest; redesenhar o mesmo quadro
não acelera a animação, e os comandos originais não são atrasados.

Os dois estilos conservam a proporção original de 240×160. A ampliação usa
passos inteiros: no modo Suave, também os subpixels do filtro têm tamanho
uniforme. Isso pode deixar margens em telas com outras resoluções. No POCO
2772×1280, ambos exibem o jogo em 1920×1280, centralizado. O filtro atua só
na apresentação, depois da correção de cores; não altera ROM, save, raster
do GBA ou hashes de estado. A interface e os controles permanecem nítidos.
O filtro não se acumula sobre o raster HD opcional do upstream.

O save normal do cartucho é local (`saves/minishcap.sav`, EEPROM de 8 KiB).
Europe usa `saves/minishcap_eu.sav`. ROMs importadas, estados rápidos,
suspensão automática e caches também usam identidades separadas por região;
não copie estados rápidos entre elas. A atualização preserva os caminhos USA
existentes. **Exportar save** exporta o cartucho da região selecionada.
Use **Exportar save** para guardar uma cópia fora do aplicativo. Estados
rápidos são separados desse save. Desinstalar o aplicativo apaga seus dados
privados; uma atualização assinada com a mesma chave preserva esses dados.
O processo de importação não envia sua ROM para servidores. O APK de release
não solicita acesso à rede nem permissão geral de armazenamento.

## Identidade dos dados

| Arquivo | Tamanho | SHA-1 |
|---|---:|---|
| The Minish Cap (USA) | 16.777.216 | `b4bd50e4131b027c334547b4524e2dbbd4227130` |
| The Minish Cap (Europe, BZMP rev. 0) | 16.777.216 | `cff199b36ff173fb6faf152653d1bccf87c26fb7` |
| Cult of GBA BIOS substituta | 16.384 | `2e41f9d30ecdf1c4540edf89ac16f49bf327edb7` |

O APK contém a BIOS **aberta**, sob MIT, e seu aviso de licença. Não contém
uma BIOS proprietária da Nintendo nem o arquivo ROM do jogo. A BIOS aberta
tem um corpus separado e os vetores SWI/IRQ próprios; a configuração da BIOS
original usada pelo upstream permanece disponível para desenvolvedores.

A ROM Europe verificada tem CRC32 `E8637292` e MD5
`2af78edbe244b5de44471368ae2b6f0b`. Apenas seus símbolos e informações de
layout são versionados em `symbols/eu/`. Código, tabelas de execução,
rotinas copiadas para RAM e mapas de depuração são selecionados juntos;
a verificação SHA-1 continua obrigatória antes da execução.

Base: [MinishCapRecomp](https://github.com/mstan/MinishCapRecomp), commit
`63993a51fb6c6410730c96f37cfb03f7e087c001`, e os submódulos fixados no Git.
É a mesma família do [FireRedLeafGreenRecomp](https://github.com/mstan/FireRedLeafGreenRecomp)
solicitado como referência. Os símbolos do Zelda vêm do trabalho do
[zeldaret/tmc](https://github.com/zeldaret/tmc); não se usa o jogo Pokémon
como substituto do código Zelda.

## Compilação

Dependências testadas: JDK 21, Python 3.11+, CMake/Ninja e compilador host C++20;
Android SDK 35, build-tools 35.0.0, NDK 28.2.13676358 e CMake do SDK 3.31.6.
O wrapper fixa Gradle 8.12 e o projeto fixa AGP 8.7.3.

```bash
git clone --recursive https://github.com/GenissonEmilio/ZeldaMinishCapAndroid.git
cd ZeldaMinishCapAndroid
cmake -S gbarecomp -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/host --target gba_recompile -j2
python3 tools/android/prepare.py --rom '/caminho/MinishCap-USA.zip' --rom '/caminho/MinishCap-Europe.zip'
export ANDROID_HOME='/caminho/android-sdk'
tools/android/build.sh Release
```

O APK sai em `android/app/build/outputs/apk/release/app-release.apk`.
O APK público contém os dois corpora e exige gerar ambos. É possível gerar
uma região de cada vez; a geração preserva o corpus da outra região. As duas
ROMs são entradas locais de compilação e nunca são empacotadas no APK público.
`-PminishRegionTest=true` cria o pacote separado `.regiontest`, com label
**Minish Cap - EU teste** e depuração habilitada para os testes ADB; não use
essa opção no APK público.
Para validar o álbum sem substituir uma instalação existente, acrescente
`-PminishAlbumTest=true` ao comando Gradle. Esse APK aparece como
**Minish Cap - Album teste** e usa um armazenamento separado (`.albumtest`);
importe a ROM nessa instalação. A opção não altera o pacote normal.
O script de geração verifica hashes, aplica `symbols/minishcap_reviewed.toml`
e conserva o mtime dos arquivos cujo conteúdo não mudou, para reaproveitar a
compilação incremental. O corpus fica ignorado em `generated/`: não o envie
ao Git. Nunca corrija o código gerado manualmente.

A geração usa orçamento de 100.000 funções, acima do limite inicial de
bring-up do motor. Isso permite concluir a descoberta automática de callbacks
e continuações na RAM depois das raízes obrigatórias do Zelda; o orçamento
anterior deixava código descoberto pendente. O script confere as continuações
observadas em teste antes de substituir o corpus existente.

Para atualizar os símbolos EU, compile a revisão fixada de `third_party/tmc`
com `GAME_VERSION=EU`, usando sua cópia em `baserom_eu.gba`. Após preparar
agbcc e `make tools` conforme o upstream, execute `make -j2 GAME_VERSION=EU`.
Então execute `python3 tools/import_tmc_symbols/import_eu_symbols.py`. O
importador recusa uma revisão diferente ou uma ROM reconstruída cujo SHA-1
não coincida; usa apenas o ELF e o mapa de link para extrair metadados.
As cópias IWRAM, seus modos ARM/THUMB e intervalos de retomada são derivados
dos símbolos ELF, incluindo a distinção entre código e literal pools.

Se preferir JDK e ferramentas host isolados:

```bash
docker build -t minishcap-tools -f tools/android/Dockerfile .
docker run --rm --cpus=2 --memory=3g --memory-swap=3584m --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$PWD:/src:z" -w /src minishcap-tools \
  sh -c 'cmake -S gbarecomp -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build/host --target gba_recompile -j2'
docker run --rm --cpus=2 --memory=3g --memory-swap=3584m --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$PWD:/src:z" -w /src minishcap-tools \
  python3 tools/android/prepare.py --rom roms/minishcap_usa.gba --rom roms/minishcap_eu.gba
docker run --rm --cpus=2 --memory=3g --memory-swap=3584m --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -e ANDROID_HOME=/sdk -e ANDROID_USER_HOME=/src/artifacts/android-user-home \
  -e GRADLE_USER_HOME=/src/artifacts/gradle-home \
  -v "$PWD:/src:z" -v "$ANDROID_HOME:/sdk:z" -w /src minishcap-tools \
  tools/android/build.sh Release
```

No Fedora, `:z` permite acesso do contêiner aos arquivos por SELinux. O Docker
não recebe acesso USB; instalação e testes ADB acontecem no host.

### Assinatura e atualizações

Configure uma chave estável antes de distribuir. O template aceita
`android/signing.local.properties` (ignorado no Git):

```properties
storeFile=/caminho/absoluto/minishcap.jks
storePassword=SUA_SENHA
keyAlias=minishcap
keyPassword=SUA_SENHA
```

O caminho deve existir no ambiente que executa Gradle (dentro do contêiner,
se for usado). Não compartilhe esse arquivo nem a chave. Guarde backup fora
da pasta de build; perder a chave impede atualizar instalações existentes.
Sem configuração, o template usa uma chave de debug e avisa no build.

## Validação reproduzível

Na v0.4.0, a ROM EU foi testada em um POCO M8 Pro 5G: importação pelo
seletor Android do ZIP original, seleção de Español, criação de arquivo,
introdução e primeira cena com diálogos em espanhol. Amostras de 20 segundos
mediram aproximadamente 59,3–59,4 FPS de guest, com zero misses, zero instruções
interpretadas e sem recompilação dinâmica em modo estrito. A suspensão EU foi
retomada, e iniciar a USA preservou byte a byte seu save EU de 8 KiB. Um
smoke USA de 10 segundos também passou em modo estrito. Esses percursos
curtos não validam toda a campanha. O APK público foi verificado sem ROM,
sem permissão INTERNET, sem depuração e com a mesma assinatura da v0.3.1.

O workflow `Android source validation` verifica a integridade do firmware,
ausência de arquivos privados versionados, sintaxe dos scripts e o importador
Java com entradas inválidas, ZIP truncado, tamanho/hash errados e limite de
descompressão. Testes também verificam que falhas preservam a importação
anterior e não deixam arquivos temporários. CI não recebe ROM nem BIOS Nintendo;
o build completo requer que o proprietário forneça a ROM localmente.

```bash
python3 tools/android/check_sources.py
mkdir -p build/import-test
javac -d build/import-test android/app/src/main/java/io/github/genissonemilio/minishcap/RomImporter.java tools/android/RomImporterTest.java
java -cp build/import-test io.github.genissonemilio.minishcap.RomImporterTest roms/minishcap_usa.gba roms/minishcap_eu.gba
python3 tools/android/audit_apk.py android/app/build/outputs/apk/release/app-release.apk
```

Builds de debug aceitam `--ez qa true` na Activity inicial. Esse modo habilita
somente o observador local e direciona o áudio para saída dummy durante testes
automáticos. Builds de release ignoram esse sinalizador. Com a partida aberta:

```bash
adb shell am start -n io.github.genissonemilio.minishcap/.ZeldaSetupActivity --ez qa true
python3 tools/android/device_probe.py --seconds 20
python3 tools/android/validate_touch.py
```

O relatório compara quadros do jogo com tempo monotônico real e consulta o
contador de misses do recompiler. FPS de guest não é taxa de atualização do
painel. Qualquer ponte por interpretação/cache nativo deve ser reportada;
nenhum teste curto prova cobertura estática de toda a campanha. O recurso de
visão ampliada do upstream continua opcional e desligado por padrão.

O adaptador em `cmake/AndroidRenderer.cmake` prepara cópias dos dois fontes
de apresentação do motor, com âncoras exatas que exigem revisão quando o
submódulo muda. O checkout original e o código derivado da ROM ficam
intactos. `src/pixel_filter.h` implementa independentemente as regras
publicadas de [Scale2x](https://www.scale2x.it/algorithm).

O workflow também verifica os 512 padrões binários de vizinhança, bordas,
cores RGB, escalas em várias resoluções e a saída real do renderer SDL por
software, alternando os modos. Para executar esse teste sem ROM:

```bash
mkdir -p build/pixel-test
g++ -std=c++17 -O2 -DMINISH_TEST_SDL -Isrc tests/pixel_filter_test.cpp src/pixel_presenter.cpp $(sdl2-config --cflags --libs) -o build/pixel-test/check
SDL_VIDEODRIVER=dummy build/pixel-test/check
```

Esse teste também confere a captura antes das camadas de interface e o
reconhecimento restrito do inventário. A persistência do álbum pode ser
validada sem Android nem ROM:

```bash
mkdir -p build/album-test
javac -d build/album-test android/app/src/main/java/io/github/genissonemilio/minishcap/CaptureStore.java tools/android/CaptureStoreTest.java
java -cp build/album-test io.github.genissonemilio.minishcap.CaptureStoreTest
```

A navegação e o estado da nova página têm um teste independente de ROM/SDL:

```bash
mkdir -p build/photo-test
g++ -std=c++17 -O2 -Wall -Wextra -Isrc tests/photo_menu_test.cpp src/photo_menu.cpp -o build/photo-test/check
build/photo-test/check
```

Ele verifica entrada pelos dois lados, propriedade dos comandos, retorno ao
menu nativo, paginação, respostas assíncronas antigas, limites e igualdade
byte a byte entre a cena recebida e a captura entregue ao armazenamento.

Para repetir o teste com interpretação/cache desativados, acrescente
`--ez qa_strict true` ao comando que inicia a tela principal. Os registros
existentes do motor ficam em `files/diagnostics/native.log`, no armazenamento
privado; builds de debug permitem consultá-los por `adb shell run-as`.

As evidências do aparelho e as limitações observadas ficam em
[VALIDATION_ANDROID.md](VALIDATION_ANDROID.md).
