import serial.tools.list_ports
import serial
import time
import os
import psutil
import re
import glob
con = serial.Serial()

mac_esp32 = 'XX:XX:XX:XX:XX:XX' # SERIAL_NUMBER_ESP32
portEsp32 = '/dev/rfcomm0'      # SERIAL_PORT
canal = 1

def conectar_rfcomm():
    resultado = os.system(f"sudo rfcomm bind 0 {mac_esp32} {canal}")  # Vincula el dispositivo
    os.system("sudo rfcomm release 0")  # Libera el puerto rfcomm antes de vincular
    return resultado == 0
    # Terminal mostrar: sdptool browse C0:49:EF:E5:4F:46
def newConexion():
    time.sleep(1)
    puertos = serial.tools.list_ports.comports()
    rfcomm_ports = []

    for puerto in puertos:
        if 'rfcomm' in puerto.device:
            rfcomm_ports.append(puerto.device)

    print("Puertos BT: ",rfcomm_ports)
    if rfcomm_ports:
        return rfcomm_ports[-1]  # Devuelve el último puerto de la lista
    else:
        return portEsp32

#---------------
def sendData(temp, rpm, gpu, free_disk, free_mem, procs):
    global con, portEsp32
    try:        
        data = f"{temp},{rpm},{free_mem},{free_disk},{gpu},{procs}/"
        con = serial.Serial(portEsp32, baudrate=115200, timeout=3)
        con.write(data.encode())
        print("Enviado: ",data.encode())
        time.sleep(1)

        if con.in_waiting > 0:
            resp = con.readline().decode(errors='replace').strip()
            print("ESP32: ", resp)
        else:
            print("Sin respuesta ESP32")

    except Exception as e:
        print(f"ERROR: {e}")
        con.close()
        time.sleep(1)
        portEsp32 = newConexion()
        conectar_rfcomm()

def GPU_Temp():
    # Intentar obtener la temperatura de la GPU (si es una NVIDIA)
    gpu_temp = os.popen("nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader").read().strip()
    return gpu_temp if gpu_temp else "n/a"
def CPU_Temp():
    temperatures = {}
    
    # Buscar en todas los sensores de temperatura
    for path in glob.glob("/sys/class/thermal/thermal_zone*/temp"):
        try:
            # Obtener el tipo de sensor
            sensor_path = path.replace("/temp", "/type")
            with open(sensor_path, "r") as f:
                sensor_name = f.read().strip()

            # Leer la temperatura
            with open(path, "r") as f:
                temp = int(f.read().strip()) / 1000  # Convertir a grados Celsius

            temperatures[sensor_name] = temp
        except:
            return "n/a"
        
    #print(temperatures) # Ver todas las temeperaturas
    return temperatures.get("x86_pkg_temp", "n/a") # ELEGIR QUE TEMPERATURA MOSTRAR, x86_pkg_temp es del CPU
def FAN_Speed():
    # Obtener la velocidad del ventilador usando lm-sensors
    fan_speed = os.popen("sensors | grep 'fan'").read().strip()

    # Buscar todos los números en la salida
    numbers = re.findall(r"\b\d+\b", fan_speed)

    # Filtrar los valores mayores a 0 y devolver el primero encontrado
    valid_numbers = [int(num) for num in numbers if int(num) > 0]

    return str(valid_numbers[0]) if valid_numbers else "n/a"
def free_Disk():
    obj_Disk = psutil.disk_usage('/') # Rutas de montaje /, /home, /mnt/data
    free_disk = int(obj_Disk.free / (1000.0 ** 3))  # GB
    return free_disk
def free_Memory():
    free_mem = int((psutil.virtual_memory().total - psutil.virtual_memory().used) / (1024 * 1024))  # MB
    return free_mem

#---------------
while True:
    try:        
        temp = CPU_Temp()
        rpm = FAN_Speed()
        disk = free_Disk()
        free_mem = free_Memory()
        proc_counter = len(list(psutil.process_iter()))
        sendData(temp, rpm, temp, disk, free_mem, proc_counter)

    except Exception:
        print("🔴 Sin comunicacion.\n")
    time.sleep(10)
