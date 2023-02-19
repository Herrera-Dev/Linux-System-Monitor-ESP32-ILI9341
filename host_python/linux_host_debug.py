import serial
import time
import os
import psutil
import re
import glob

#connection = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=1)
#time.sleep(2)
ultimo_envio = time.time()

def sendData(temp, rpm, gpu, free_disk, free_mem, procs): # SELECCIONAR MODO DE ENVIO DE DATOS
    try:
        #connection = serial.Serial('/dev/rfcomm0', baudrate=115200, timeout=1) # Port Bluetooth
        connection = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=2)
        data = f"{temp},{rpm},{free_mem},{free_disk},{gpu},{procs}/"
        time.sleep(1)
        connection.write(data.encode())
        
        print("Datos enviados:", data.encode())  
        connection.close()     

    except Exception as e:
        print("Error:", e)

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

while True:
    try:
        tiempo_actual = time.time()
        if tiempo_actual - ultimo_envio >= 10:
            temp = CPU_Temp()
            rpm = FAN_Speed()
            disk = free_Disk()
            
            free_mem = int((psutil.virtual_memory().total - psutil.virtual_memory().used) / (1024 * 1024))  # MB
            proc_counter = len(list(psutil.process_iter()))
                
            print("--------------")
            sendData(temp, rpm, temp, disk, free_mem, proc_counter)
            ultimo_envio = tiempo_actual

        connection = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=1)
        time.sleep(1)
        if connection.in_waiting > 0:
            respuesta = connection.readline().decode(errors='replace').strip()
            if respuesta:
                print("🔹 ESP32:", respuesta)
        time.sleep(0.1)
        connection.close()

    except Exception:
        print("\n🔴 Finalizando comunicación")
        connection.close()
        break
