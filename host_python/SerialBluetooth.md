# 📘 Comandos básicos de `bluetoothctl`

## 🔧 Comandos básicos

| Comando     | Descripción                                                   |
|-------------|---------------------------------------------------------------|
| `power on`  | Enciende el adaptador Bluetooth.                              |
| `power off` | Apaga el adaptador Bluetooth.                                 |
| `agent on`  | Activa el agente para permitir emparejamientos.               |
| `default-agent` | Establece el agente como predeterminado.                  |
| `scan on`   | Comienza a escanear dispositivos Bluetooth cercanos.          |
| `scan off`  | Detiene el escaneo de dispositivos.                           |
| `devices`   | Lista todos los dispositivos detectados.                      |
| `paired-devices` | Muestra los dispositivos ya emparejados.                 |
| `info XX:XX:XX:XX:XX:XX` | Muestra información detallada sobre un dispositivo. |

---

## 🔗 Emparejar y conectar

| Comando     | Descripción                                                   |
|-------------|---------------------------------------------------------------|
| `pair XX:XX:XX:XX:XX:XX`      | Empareja el dispositivo con esa MAC.       |
| `trust XX:XX:XX:XX:XX:XX`     | Marca el dispositivo como confiable (necesario para conexiones automáticas). |
| `connect XX:XX:XX:XX:XX:XX`   | Intenta conectarse al dispositivo.         |
| `disconnect XX:XX:XX:XX:XX:XX`| Desconecta el dispositivo.                 |
| `remove XX:XX:XX:XX:XX:XX`    | Elimina (desempareja) el dispositivo.      |

---

## 📦 Servicios y ayuda

| Comando     | Descripción                                                   |
|-------------|---------------------------------------------------------------|
| `list`      | Muestra los adaptadores Bluetooth disponibles.               |
| `select XX:XX:XX:XX:XX:XX` | Selecciona un adaptador específico.           |
| `exit` o `quit` | Sale del modo interactivo de `bluetoothctl`.             |
| `help`      | Muestra todos los comandos disponibles en `bluetoothctl`.     |

## Problemas
Al iniciar el sistema no se crea el puerto serial virtual `/dev/rfcomm0` automaticamente hay que iniciarlo manualmente como adm o hacer algun script para automatizarlo:
`sudo rfcomm bind 0 XX:XX:XX:XX:XX:XX 1`
`sudo rfcomm release 0`
Caso contrario el script no podra conectarse al bluetooth.
