import serial.tools.list_ports
import serial
import time
import os
import psutil
import re
import glob

ultimo_envio = time.time()
serialEsp32 = "0001" # Serial Number ESP32 --> serial_dispositivos.py
portEsp32 = None
con = serial.Serial()

def leerSerial(con):
    try:
        con = serial.Serial(portEsp32, baudrate=115200)
        con.rts = False
        con.dtr = False
        if con.in_waiting > 0:
            resp = con.readline().decode(errors='replace').strip()
            print("ESP32: ", resp)
        con.close()
    except Exception as e:
        con.close()
        print(f"ERROR:", {e})
        newConexion()
def encontrarPuerto():
    puertos = serial.tools.list_ports.comports()
    dispositivos = {}

    for puerto in puertos:
        dispositivos[puerto.serial_number] = puerto.device

    if dispositivos:
        for id, puerto in dispositivos.items():
            #print(f" - {id} en: {puerto}")
            if serialEsp32 == id:
                return puerto
    else:
        print("No se encontraron dispositivos")
    return None
def newConexion():
    global portEsp32, con
    con.close()

    portEsp32 = encontrarPuerto()
    while portEsp32 == None:
        portEsp32 = encontrarPuerto()
        time.sleep(3)

#---------------
def sendData(temp, rpm, gpu, free_disk, free_mem, procs): # SELECCIONAR MODO DE ENVIO DE DATOS
    global con
    try:
        if portEsp32 is not None:
            data = f"{temp},{rpm},{free_mem},{free_disk},{gpu},{procs}/"
            con = serial.Serial(portEsp32, baudrate=115200) # Port USB
            con.rts = False # Evitar que el ESP32 se reinicie
            con.dtr = False # Evitar que el ESP32 se reinicie
            con.write(data.encode())
            print("Enviado:", data.encode())  
            con.close()
        else:
            newConexion() 

    except Exception as e:        
        con.close()
        print(f"ERROR:", {e})
        newConexion()

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

#----------------
newConexion()
print("Enviando datos a ESP32...")

while True:
    try:
        tiempo_actual = time.time()
        if tiempo_actual - ultimo_envio > 10:
            temp = CPU_Temp()
            rpm = FAN_Speed()
            disk = free_Disk()
            free_mem = free_Memory()
            proc_counter = len(list(psutil.process_iter()))
            sendData(temp, rpm, temp, disk, free_mem, proc_counter)
            ultimo_envio = tiempo_actual
        
        #leerSerial(con) # Leer puerto serial de la ESP32
        time.sleep(0.2)

    except Exception:
        print("🔴 Sin comunicación.\n")
        con.close()
        newConexion()
