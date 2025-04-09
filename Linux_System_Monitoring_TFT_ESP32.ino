
// ADAPTADO PARA PANTALLA ILI9341 Y ESP32

// Framework ESP32 v3.1.3
#include <TFT_eSPI.h> //2.5.43
#include <SPIFFS.h>
#include <FS.h>
#include "BluetoothSerial.h" //v1.1.0

// ¡Elija uno de estos dependiendo de su configuración!
//-------------- SELECCIONAR --------------------
#define ENABLE_RES_TOUCH // devkitc con ili9488 usa RES_TOUCH
// #define NO_TOUCH      // usa NO_TOUCH cuando tu pantalla no tiene toque

//------------------------------------------
// Seleccionar modo BLUETOOTH o USB UART
#define MODO_BLUETOOTH 0
const int redrawtime = 10000; // Cambiar esto para que coincida con el script en el host
const int tDesc = 300000;     // 5 min. Restablecer si no hay nuevos valores

const int x1 = 50;
const int x2 = 160;
const int x3 = 270;
const int yy1 = 100;
const int yy2 = 200;

// La escala del eje y para el gráfico
const int ymax_cpu = 100;   // In Celcius
const int ymax_fan = 2000;  // In rpm
const int ymax_ram = 8192;  // In MB
const int ymax_hdd = 57;    // In GB
const int ymax_gpu = 100;   // In Celcius
const int ymax_procs = 500; // In # (numbers)

// Defina los niveles de advertencia, cambie esto como quieras
const int warn_cpu = 70;    // Higher than, in C
const int warn_rpm = 7000;  // Higher than, in RPM
const int warn_ram = 2000;  // Lower than, in MB
const int warn_hdd = 35;    // Lower than, in GB
const int warn_gpu = 70;    // Higher than, in Celcius
const int warn_procs = 800; // Higher than, in #

// lugar para almacenar las últimas 21 lecturas
int32_t cpu[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int32_t fan[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int32_t ram[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int32_t hdd[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int32_t gpu[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int32_t procs[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Pantalla ILI9341 TFT
int screenwidth = 320;
int screenheight = 240;
int margin = 40;

boolean graphshowing = false;
boolean homescreen = true;
int currentgraph = 0;
long previousmillis = -10;
long tAntConect = 0;

#define FILESYSTEM SPIFFS                   // Defina el sistema de archivos que estamos utilizando
#define CALIBRATION_FILE "/calibrationData" // Defina el nombre de nuestro archivo de calibración

// Creación de objetos
TFT_eSPI tft = TFT_eSPI();
#if MODO_BLUETOOTH
BluetoothSerial BTSerial;
#endif

// -----------------------------------
void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial begin");

#if MODO_BLUETOOTH
  BTSerial.begin("MonitorESP32"); // Iniciar Bluetooth Serial
#endif

  // Comenzar nuestro sistema de archivos
  if (FILESYSTEM.begin())
  {
    Serial.println("SPIFFS begin");
  }
  else
  {
    Serial.println("SPIFFS error");
  }

  // Inicializar TFT
  tft.begin();
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);
  // tft.setFreeFont(&FreeSansBold12pt7b);
  tft.setTextColor(TFT_SKYBLUE);
  tft.setTextFont(1);
  tft.setTextSize(2);

#ifdef ENABLE_RES_TOUCH
  touch_calibrate();
#endif

  // Dibujar
  drawBmp("/bg.bmp", 0, 0);
  Serial.println("Fondo dibujado.");

  tft.drawCentreString("n/a", x1, yy1, 1);
  tft.drawCentreString("n/a", x2, yy1, 1);
  tft.drawCentreString("n/a", x3, yy1, 1);

  tft.drawCentreString("n/a", x1, yy2, 1);
  tft.drawCentreString("n/a", x2, yy2, 1);
  tft.drawCentreString("n/a", x3, yy2, 1);

  Serial.println("Configuracion terminada.");
}

void loop()
{
  // Algunas variables que necesitamos
  int t_y, t_x;
  uint16_t x, y;
  bool pressed = false;
  pressed = false; // Asegúrese de que no usemos el toque de Last Loop

// Verifique los datos en serie entrantes
// Esperamos datos entrantes como este: "33,428,8343,16000,68,371" (temp, rpm, mem_use, free_disk, gpu, procs)
#if MODO_BLUETOOTH
  if (BTSerial.available())
  {
    BTSerial.println("OK");
    String cpu_string = BTSerial.readStringUntil(',');
    String fan_string = BTSerial.readStringUntil(',');
    String ram_string = BTSerial.readStringUntil(',');
    String hdd_string = BTSerial.readStringUntil(',');
    String gpu_string = BTSerial.readStringUntil(',');
    String procs_string = BTSerial.readStringUntil('/');

    // Agregar valores que tenemos a nuestras matrices
    addToArray(cpu_string, cpu);
    addToArray(fan_string, fan);
    addToArray(ram_string, ram);
    addToArray(hdd_string, hdd);
    addToArray(gpu_string, gpu);
    addToArray(procs_string, procs);
    tAntConect = millis();
  }
#else
  if (Serial.available())
  {
    Serial.print("Datos recibidos");
    String cpu_string = Serial.readStringUntil(',');
    String fan_string = Serial.readStringUntil(',');
    String ram_string = Serial.readStringUntil(',');
    String hdd_string = Serial.readStringUntil(',');
    String gpu_string = Serial.readStringUntil(',');
    String procs_string = Serial.readStringUntil('/');

    // Agregar valores que tenemos a nuestras matrices
    addToArray(cpu_string, cpu);
    addToArray(fan_string, fan);
    addToArray(ram_string, ram);
    addToArray(hdd_string, hdd);
    addToArray(gpu_string, gpu);
    addToArray(procs_string, procs);
    tAntConect = millis();
  }
#endif

  if (tAntConect + tDesc < millis())
  {
    addToArray("0", cpu);
    addToArray("0", fan);
    addToArray("0", ram);
    addToArray("0", hdd);
    addToArray("0", gpu);
    addToArray("0", procs);
  }

#ifdef ENABLE_RES_TOUCH
  if (tft.getTouch(&x, &y))
  {
    t_x = x;
    t_y = y;
    pressed = true;
  }
#endif

  // Toques en pantalla
  if (pressed)
  {
    if (graphshowing)
    {
      drawBmp("/bg.bmp", 0, 0);
      updateHomeScreen();
      graphshowing = false;
    }
    else
    {
      if (t_y < screenheight / 2)
      {
        Serial.print("Upper ");

        if (t_x < screenwidth / 3)
        {
          Serial.println("Left");
          drawGraph(0, sizeof(cpu) / sizeof(cpu[0]), 0, ymax_cpu, cpu, "TEMPERATURA DE LA CPU", false);
          graphshowing = true;
          currentgraph = 1;
        }
        else if (t_x < (screenwidth / 3) * 2)
        {
          Serial.println("Middle");
          drawGraph(0, sizeof(fan) / sizeof(fan[0]), 0, ymax_fan, fan, "VELOCIDAD DEL VENTILADOR (RPM)", false);
          graphshowing = true;
          currentgraph = 2;
        }
        else
        {
          Serial.println("Right");
          drawGraph(0, sizeof(ram) / sizeof(ram[0]), 0, ymax_ram, ram, "MEMORIA RAM LIBRE (MB)", false);
          graphshowing = true;
          currentgraph = 3;
        }
      }
      else
      {
        Serial.print("Lower ");
        if (t_x < screenwidth / 3)
        {
          Serial.println("Left");
          drawGraph(0, sizeof(hdd) / sizeof(hdd[0]), 0, ymax_hdd, hdd, "ESPACIO LIBRE (GB)", false);
          graphshowing = true;
          currentgraph = 4;
        }
        else if (t_x < (screenwidth / 3) * 2)
        {
          Serial.println("Middle");
          drawGraph(0, sizeof(gpu) / sizeof(gpu[0]), 0, ymax_gpu, gpu, "TEMPERATURA DE LA CPU", false);
          graphshowing = true;
          currentgraph = 5;
        }
        else
        {
          Serial.println("Right");

          drawGraph(0, sizeof(procs) / sizeof(procs[0]), 0, ymax_procs, procs, "NUMERO DE PROCESOS ACTIVOS", false);
          graphshowing = true;
          currentgraph = 6;
        }
      }
      pressed = false;
      delay(300);
    }
  }

  // Redibuje los gráficos si es necesario
  if (graphshowing && previousmillis + redrawtime <= millis())
  {
    previousmillis = millis();
    if (currentgraph == 1)
    {
      drawGraph(0, sizeof(cpu) / sizeof(cpu[0]), 0, ymax_cpu, cpu, "TEMPERATURA DE LA CPU", true);
    }
    else if (currentgraph == 2)
    {
      drawGraph(0, sizeof(fan) / sizeof(fan[0]), 0, ymax_fan, fan, "VELOCIDAD DEL VENTILADOR (RPM)", true);
    }
    else if (currentgraph == 3)
    {
      drawGraph(0, sizeof(ram) / sizeof(ram[0]), 0, ymax_ram, ram, "MEMORIA RAM LIBRE (MB)", true);
    }
    else if (currentgraph == 4)
    {
      drawGraph(0, sizeof(hdd) / sizeof(hdd[0]), 0, ymax_hdd, hdd, "ESPACIO LIBRE (GB)", true);
    }
    else if (currentgraph == 5)
    {
      drawGraph(0, sizeof(gpu) / sizeof(gpu[0]), 0, ymax_gpu, gpu, "TEMPERATURA DE LA GPU", true);
    }
    else if (currentgraph == 6)
    {
      drawGraph(0, sizeof(procs) / sizeof(procs[0]), 0, ymax_procs, procs, "NUMERO DE PROCESOS ACTIVOS", true);
    }
  }
  else if (previousmillis + redrawtime <= millis())
  {
    previousmillis = millis();
    updateHomeScreen();
  }
  delay(10);
}

// -----------------------------------
// Función para cambiar todos los valores en una matriz a la izquierda y agregar un valor
void addToArray(String addvalue, int32_t *arr)
{
  int32_t n = addvalue.toInt();

  for (int i = 0; i < 20; i++)
  {
    arr[i] = arr[i + 1];
  }

  if (n > 0)
  {
    arr[20] = addvalue.toInt();
  }
  else
  {
    arr[20] = 1;
    delay(10000);
    ESP.restart();
  }
}

// Dibuja la pantalla de inicio
void updateHomeScreen()
{
  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.setTextColor(TFT_SKYBLUE);
  tft.setTextSize(1);

  // Haz algunas buenas cuerdas para imprimir
  String temp_data = String(cpu[20]) + " C";
  String rpm_data = String(fan[20]) + " RPM";
  String ram_data = String(ram[20]) + " MB";

  String hdd_data = String(hdd[20]) + " GB";
  String gpu_data = String(gpu[20]) + " C";
  String procs_data = String(procs[20]);

  // Borrar valores dibujando un rectángulo negro
  tft.fillRect(15, 94, 320, 30, TFT_BLACK);
  tft.fillRect(15, 200, 320, 30, TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(2);

  if (cpu[20] > warn_cpu)
  {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(temp_data, x1, yy1, 1);
  }
  else
  {
    tft.setTextColor(TFT_SKYBLUE);
    tft.drawCentreString(temp_data, x1, yy1, 1);
  }

  if (fan[20] > warn_rpm)
  {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(rpm_data, x2, yy1, 1);
  }
  else
  {
    tft.setTextColor(TFT_SKYBLUE);
    tft.drawCentreString(rpm_data, x2, yy1, 1);
  }

  if (ram[20] < warn_ram)
  {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(ram_data, x3, yy1, 1);
    ;
  }
  else
  {
    tft.setTextColor(TFT_SKYBLUE);
    tft.drawCentreString(ram_data, x3, yy1, 1);
  }

  if (hdd[20] < warn_hdd)
  {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(hdd_data, x1, yy2, 1);
  }
  else
  {
    tft.setTextColor(TFT_SKYBLUE);
    tft.drawCentreString(hdd_data, x1, yy2, 1);
  }

  if (gpu[20] > warn_gpu)
  {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(gpu_data, x2, yy2, 1);
  }
  else
  {
    tft.setTextColor(TFT_SKYBLUE);
    tft.drawCentreString(gpu_data, x2, yy2, 1);
  }

  if (procs[20] > warn_procs)
  {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(procs_data, x3, yy2, 1);
  }
  else
  {
    tft.setTextColor(TFT_SKYBLUE);
    tft.drawCentreString(procs_data, x3, yy2, 1);
  }
}

// funciona para dibujar un gráfico
void drawGraph(int32_t xmin, int32_t xmax, int32_t ymin, int32_t ymax, int32_t *arr, String title, bool redraw)
{

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setTextFont(1);

  int32_t arrlen = xmax;

  if (redraw)
  {

    tft.fillRect(0 + margin + 1, 0 + margin - 1, screenwidth + 5 - (2 * margin), screenheight - (2 * margin) - 1, TFT_BLACK);
    tft.drawLine(0 + margin, 0 + margin, 0 + margin, screenheight - margin, TFT_SKYBLUE);
    tft.drawLine(0 + margin, screenheight - margin, screenwidth - margin, screenheight - margin, TFT_SKYBLUE);
  }

  if (!redraw)
  { // if reDraw, do not draw the x/y axis again

    // If this is the first draw, black the screen first
    tft.fillScreen(TFT_BLACK);

    // Draw graph title
    tft.setTextDatum(ML_DATUM);
    tft.drawString(title, margin - 10, margin - 20, 1);

    // Draw empty graph
    tft.drawLine(0 + margin, 0 + margin, 0 + margin, screenheight - margin, TFT_SKYBLUE);
    tft.drawLine(0 + margin, screenheight - margin, screenwidth - margin, screenheight - margin, TFT_SKYBLUE);

    tft.setTextDatum(MR_DATUM);
    tft.drawString("0", margin - 5, (screenheight - margin) + 10, 1);
  }

  // Draw the Y-axis
  int32_t yrange = ymax - ymin;
  int32_t ysteps = (screenheight - 2 * margin) / 5;

  if (!redraw)
  { // if reDraw, do not draw the x/y axis again

    for (int i = 1; i < 5; i++)
    {
      tft.setTextDatum(MR_DATUM);
      tft.drawNumber(0 + ((yrange / 5) * i), margin - 5, (screenheight - margin) - ysteps * i, 1);
    }

    tft.drawNumber(ymax, margin - 5, (screenheight - margin) - ysteps * 5, 1);
  }

  // Draw the X-axis
  int32_t xrange = xmax - xmin;
  int32_t xsteps = (screenwidth - 2 * margin) / (arrlen - 1);

  if (!redraw)
  { // if reDraw, do not draw the x/y axis again
    for (int i = 1; i < arrlen - 1; i++)
    {
      tft.setTextDatum(MC_DATUM);
      tft.drawNumber(i, (0 + margin) + xsteps * i, (screenheight - margin) + 10, 1);
    }

    tft.drawNumber(xmax - 1, (0 + margin) + xsteps * (arrlen - 1), (screenheight - margin) + 10, 1);
  }

  for (int i = 0; i < arrlen; i++)
  {

    Serial.printf("%i, ", arr[i]);
  }

  Serial.println();

  for (int i = 0; i < arrlen - 1; i++)
  {

    int32_t point = map(arr[i], ymax, ymin, 0 + margin, screenheight - margin);
    int32_t nextpoint = map(arr[i + 1], ymax, ymin, 0 + margin, screenheight - margin);

    // tft.fillCircle((0+margin)+xsteps*i, point, 1, TFT_WHITE);

    if (nextpoint < screenheight - (margin) && nextpoint > -1 && point < screenheight - (margin))
    {

      Serial.printf("%i, %i \n", point, nextpoint);

      tft.drawLine((2 + margin) + (xsteps * i), point, (2 + margin) + xsteps * (i + 1), nextpoint, TFT_WHITE);
    }
  }

  tft.setTextColor(TFT_SKYBLUE);
  tft.setTextSize(3);
  tft.setFreeFont(&FreeSansBold12pt7b);
}

// Funciones utilizadas para dibujar una imagen BMP, no importantes para usted
void drawBmp(const char *filename, int16_t x, int16_t y)
{

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (!bmpFS)
  {

    Serial.print("Archivo no encontrado:");
    Serial.println(filename);
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row;
  uint8_t r, g, b;

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Presione la fila de píxeles a la pantalla, PushImage recortará la línea si es necesario
        // y se disminuye a medida que la imagen BMP se dibuja de abajo hacia arriba
        tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
    }
    else
      Serial.println("[WARNING]: BMP format not recognized.");
  }
  bmpFS.close();
}

uint16_t read16(fs::File &f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

#ifdef ENABLE_RES_TOUCH
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // Compruebe si existe el archivo de calibración y el tamaño es correcto
  if (FILESYSTEM.exists(CALIBRATION_FILE))
  {
    fs::File f = FILESYSTEM.open(CALIBRATION_FILE, "r");
    if (f)
    {
      if (f.readBytes((char *)calData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }

  if (calDataOK)
  {
    // Datos de calibración válidos
    tft.setTouch(calData);
    Serial.println("Calibración tactil cargada.");
  }
  else
  {
    // Data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Toque las esquinas como se indica");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibracion completa!");
    Serial.println("Calibración táctil cargada.");

    // Almacenar datos
    fs::File f = FILESYSTEM.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
#endif
