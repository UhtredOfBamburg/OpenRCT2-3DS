#!/bin/bash
set -e

ROOT_DIR="/Users/pdrtorres/.gemini/antigravity/scratch/openrct2-3ds"
BUILD_DIR="${ROOT_DIR}/build_3ds"
REL_DIR="${ROOT_DIR}/release/OpenRCT2-3DS-v1.0.0"
ZIP_PATH="${ROOT_DIR}/release/OpenRCT2-3DS-v1.0.0.zip"

echo "=== Preparing Release Package: OpenRCT2-3DS v1.0.0 ==="
rm -rf "${REL_DIR}"
rm -f "${ZIP_PATH}"

mkdir -p "${REL_DIR}/CIA"
mkdir -p "${REL_DIR}/3ds/openrct2/data"
mkdir -p "${REL_DIR}/3ds/openrct2/rct1"
mkdir -p "${REL_DIR}/3ds/openrct2/rct2"
mkdir -p "${REL_DIR}/3ds/openrct2/user/save"
mkdir -p "${REL_DIR}/3ds/openrct2/user/landscape"
mkdir -p "${REL_DIR}/3ds/openrct2/user/track"
mkdir -p "${REL_DIR}/3ds/openrct2/user/themes"
mkdir -p "${REL_DIR}/3ds/openrct2/user/title sequences"

# 1. Executables
echo "Copying executables..."
cp "${BUILD_DIR}/openrct2.cia" "${REL_DIR}/CIA/openrct2.cia"
cp "${BUILD_DIR}/openrct2.3dsx" "${REL_DIR}/3ds/openrct2/openrct2.3dsx"
cp "${BUILD_DIR}/openrct2.smdh" "${REL_DIR}/3ds/openrct2/openrct2.smdh"

# Also update dist/3ds/openrct2 for direct SD card copy
mkdir -p "${ROOT_DIR}/dist/3ds/openrct2"
cp "${BUILD_DIR}/openrct2.cia" "${ROOT_DIR}/dist/3ds/openrct2/openrct2.cia"
cp "${BUILD_DIR}/openrct2.3dsx" "${ROOT_DIR}/dist/3ds/openrct2/openrct2.3dsx"
cp "${BUILD_DIR}/openrct2.smdh" "${ROOT_DIR}/dist/3ds/openrct2/openrct2.smdh"

# 2. Open-source runtime assets (GPLv3)
echo "Copying open-source data assets..."
cp "${ROOT_DIR}/engine/data/g2.dat" "${REL_DIR}/3ds/openrct2/data/g2.dat"
cp -R "${ROOT_DIR}/engine/data/language" "${REL_DIR}/3ds/openrct2/data/language"
cp -R "${ROOT_DIR}/engine/data/title" "${REL_DIR}/3ds/openrct2/data/title"
cp -R "${ROOT_DIR}/engine/data/shaders" "${REL_DIR}/3ds/openrct2/data/shaders"

# 3. Default optimized 3DS configuration
echo "Generating default 3DS config.ini..."
cat > "${REL_DIR}/3ds/openrct2/user/config.ini" << 'EOF'
[general]
always_show_gridlines = false
autosave = 2
confirmation_prompt = false
currency_format = BRL
custom_currency_rate = 10
custom_currency_affix = PREFIX
custom_currency_symbol = "R$"
edge_scrolling = false
fullscreen_mode = 0
fullscreen_height = 480
fullscreen_width = 400
game_path = "sdmc:/3ds/openrct2/rct2"
rct1_path = "sdmc:/3ds/openrct2/rct1"
landscape_smoothing = true
language = pt-BR
measurement_format = METRIC
play_intro = false
save_plugin_data = true
debugging_tools = false
show_height_as_units = false
temperature_format = CELSIUS
window_height = 480
window_snap_proximity = 5
window_width = 400
default_display = 0
drawing_engine = SOFTWARE
uncap_fps = false
use_vsync = true
date_format = DAY_MONTH_YEAR
auto_staff = true
handymen_mow_default = false
default_inspection_interval = 2
last_run_version = "0.1.2"
invert_viewport_drag = false
load_save_sort = NAME_ASCENDING
minimize_fullscreen_focus_loss = false
day_night_cycle = false
enable_light_fx = false
enable_light_fx_for_vehicles = false
upper_case_banners = false
disable_lightning_effect = true
allow_loading_with_incorrect_checksum = true
steam_overlay_pause = false
window_scale = 1.00
scale_quality = 0
use_nn_at_integer_scales = true
show_fps = false
trap_cursor = false
auto_open_shops = true
scenario_select_mode = 1
scenario_unlocking_enabled = true
scenario_hide_mega_park = true
last_game_directory = "sdmc:/3ds/openrct2/user/save"
last_landscape_directory = "sdmc:/3ds/openrct2/user/landscape"
last_scenario_directory = "sdmc:/3ds/openrct2/rct2/Scenarios"
last_track_directory = "sdmc:/3ds/openrct2/user/track"
window_limit = 16
zoom_to_cursor = true
render_weather_effects = true
render_weather_gloom = true
show_guest_purchases = false
show_real_names_of_guests = true
allow_early_completion = false

[interface]
toolbar_show_finances = true
toolbar_show_research = true
toolbar_show_cheats = false
toolbar_show_news = false
toolbar_show_mute = false
select_by_track_type = true
console_small_font = true
current_theme_preset = "*RCT1"
current_title_sequence_preset = "rct1_2_3ds"
object_selection_filter_flags = 63

[sound]
audio_device = ""
master_sound = true
master_volume = 100
title_music = 3
sound_enabled = true
sound_volume = 100
ride_music_enabled = true
ride_music_volume = 85
audio_focus = false
EOF

# 4. Instructions for RCT2 & RCT1 assets
cat > "${REL_DIR}/3ds/openrct2/rct2/COMO_INSTALAR.txt" << 'EOF'
===========================================================
  OpenRCT2 3DS - Instruções para Arquivos do RollerCoaster Tycoon 2
===========================================================
[PT-BR]
O OpenRCT2 requer os arquivos gráficos e de som do jogo RollerCoaster Tycoon 2 original (versão Steam, GOG ou CD-ROM).
Por questões legais e de direitos autorais da Atari / Chris Sawyer, esses arquivos comerciais NÃO são distribuídos com o OpenRCT2.

Copie do seu RCT2 no computador as seguintes pastas para cá:
- Data/       (contendo csg1.dat, css1.dat, etc.)
- ObjData/    (contendo os objetos .DAT do jogo)
- Scenarios/  (cenários de jogo)
- Tracks/     (montanhas-russas pré-construídas)

A estrutura final deve ser:
sdmc:/3ds/openrct2/rct2/Data/
sdmc:/3ds/openrct2/rct2/ObjData/
sdmc:/3ds/openrct2/rct2/Scenarios/
sdmc:/3ds/openrct2/rct2/Tracks/

-----------------------------------------------------------
[EN]
OpenRCT2 requires the original graphic and sound assets from RollerCoaster Tycoon 2 (Steam, GOG, or CD-ROM).
Due to copyright laws and Atari / Chris Sawyer guidelines, these proprietary assets CANNOT be distributed bundled with OpenRCT2.

Copy the following folders from your PC installation into this folder:
- Data/       (contains csg1.dat, css1.dat, etc.)
- ObjData/    (contains .DAT object files)
- Scenarios/  (game scenarios)
- Tracks/     (roller coaster tracks)

Final path structure:
sdmc:/3ds/openrct2/rct2/Data/
sdmc:/3ds/openrct2/rct2/ObjData/
sdmc:/3ds/openrct2/rct2/Scenarios/
sdmc:/3ds/openrct2/rct2/Tracks/
EOF

cat > "${REL_DIR}/3ds/openrct2/rct1/COMO_INSTALAR.txt" << 'EOF'
===========================================================
  OpenRCT2 3DS - Arquivos do RCT1 Deluxe (Opcional)
===========================================================
[PT-BR]
Se você tiver o RollerCoaster Tycoon 1 Deluxe instalado no computador, pode copiar as pastas dele para cá para jogar todas as fases e músicas originais do RCT1:
- Data/
- Scenarios/
- Tracks/

Estrutura final:
sdmc:/3ds/openrct2/rct1/Data/
sdmc:/3ds/openrct2/rct1/Scenarios/
sdmc:/3ds/openrct2/rct1/Tracks/

-----------------------------------------------------------
[EN]
If you have RollerCoaster Tycoon 1 Deluxe, copy its folders here to unlock all classic RCT1 scenarios and music on your 3DS:
- Data/
- Scenarios/
- Tracks/
EOF

# 5. License file
cp "${ROOT_DIR}/engine/licence.txt" "${REL_DIR}/licence.txt"

# 6. README.md
cat > "${REL_DIR}/README.md" << 'EOF'
# OpenRCT2 for Nintendo 3DS (Dual-Screen Edition)

Port do **OpenRCT2** (RollerCoaster Tycoon 2 & 1) totalmente adaptado para as duas telas do **Nintendo 3DS**!

---

## 🚀 Requisitos e Compatibilidade
- **Console Recomendado:** **New Nintendo 3DS**, **New 3DS XL** ou **New 2DS XL**.
  - O jogo faz uso do processador quad-core de 804 MHz, cache L2 e do modo estendido de memória RAM (124 MB) liberados pelo `.cia` / `.3dsx`.
- **Nintendo 3DS clássico / 2DS:** Pode apresentar limitações de desempenho devido à CPU de 268 MHz e limite de 64 MB de RAM.
- **Cartão SD:** Pelo menos 600 MB livres.

---

## 📦 Como Instalar

### Método 1: Instalação via CIA (Recomendado - Home Menu)
1. Extraia a pasta `3ds` deste arquivo zip para a raiz do seu cartão SD.
2. Copie o arquivo `CIA/openrct2.cia` para a pasta `cias` do seu cartão SD.
3. Insira o cartão no 3DS, abra o **FBI**, navegue até `SD -> cias -> openrct2.cia` e selecione **"Install and delete CIA"**.
4. Copie os arquivos do **RollerCoaster Tycoon 2** do seu PC (Steam / GOG / CD) para:
   `sdmc:/3ds/openrct2/rct2/` (precisa conter as pastas `Data`, `ObjData`, `Scenarios`, `Tracks`).
5. (Opcional) Copie os arquivos do **RCT1 Deluxe** para `sdmc:/3ds/openrct2/rct1/`.
6. Abra o jogo diretamente pelo ícone na tela inicial (Home Menu)!

### Método 2: Instalação via Homebrew Launcher (.3dsx)
1. Extraia o conteúdo deste zip diretamente para a raiz do seu cartão SD.
2. Certifique-se de que os arquivos do RCT2 estejam em `sdmc:/3ds/openrct2/rct2/`.
3. Abra o **Homebrew Launcher** no seu 3DS e execute o **OpenRCT2**.

---

## 🎮 Controles no 3DS

| Botão | Função |
|---|---|
| **Stylus / Touch** | Interação direta, rolagem do mapa, construção e menus |
| **Circle Pad / D-Pad** | Mover câmera pelo parque |
| **A** | Confirmar / Clique esquerdo |
| **B** | Cancelar / Fechar janelas |
| **X** | Alternar modo de visualização |
| **Y** | Pausar / Despausar simulação |
| **L** | Girar câmera para a esquerda |
| **R** | Girar câmera para a direita |
| **ZL / ZR** (New 3DS) | Zoom In / Zoom Out |
| **C-Stick** (New 3DS) | Ajuste fino da câmera |
| **START** | Menu Principal / Salvar jogo |

---

## ⚖️ Licença e Termos de Uso (GPLv3 & Copyright)
- O código-fonte do motor de jogo é distribuído sob os termos da licença **GNU General Public License v3 (GPLv3)**. Veja o arquivo `licence.txt`.
- Código-fonte completo e instruções de compilação disponíveis em:  
  [https://github.com/](https://github.com/) (Consulte o repositório oficial do projeto)
- **RollerCoaster Tycoon** é uma marca registrada de Chris Sawyer / Atari Interactive, Inc.
- Este pacote **NÃO** inclui arquivos protegidos por direitos autorais da Atari ou Chris Sawyer. É necessário que o jogador possua uma cópia legal do RollerCoaster Tycoon 2.

EOF

# 7. Create ZIP archive
echo "Creating ZIP archive..."
cd "${ROOT_DIR}/release"
zip -r "OpenRCT2-3DS-v1.0.0.zip" "OpenRCT2-3DS-v1.0.0"

echo "=== Release package created successfully! ==="
ls -lh "${ZIP_PATH}"

