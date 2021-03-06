#include<ESP8266WiFi.h> //Biblioteca para funcionamento do WiFi do ESP
#include<ESP8266WebServer.h> //Biblioteca para o ESP funcionar como servidor
#include <DHT.h> //Biblioteca para funcionamento do sensor de temperatura e umidade DHT11
#include <FirebaseArduino.h> //Biblioteca Firebase
#include <WiFiManager.h> //Biblioteca configuração do WiFi
#include <DNSServer.h>  //Biblioteca para servidor de configuração


// Difinindo Firebase e WIFI
#define FIREBASE_HOST "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" //Host do firebase realtime database 
#define FIREBASE_AUTH "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" //Key do projeto do firebase para gravar 
const char* ssid = "XXXXXXXX";  // Rede WiFi
const char* password = "XXXXXXXX";  //Senha da Rede WiFi

ESP8266WebServer server(80); //server na porta 80

#define DHTPIN 4 //Pino digital D2 (GPIO5) conectado ao DHT11
#define DHTTYPE DHT11 //Tipo do sensor DHT11

DHT dht(DHTPIN, DHTTYPE); //Inicializando o objeto dht do tipo DHT passando como parâmetro o pino (DHTPIN) e o tipo do sensor (DHTTYPE)

float temperatura; //variável para armazenar a temperatura
float umidade; //Variável para armazenar a umidade

int tempo = 0;  //Variável para definir tempo

String mac = WiFi.softAPmacAddress();

void setup() {
  Serial.begin(9600); //Inicializa a comunicação serial
  delay(50); // Intervalo para aguardar a estabilização do sistema
  dht.begin(); //Inicializa o sensor DHT11

  Serial.println("Conectando a Rede: "); //Imprime na serial a mensagem
  Serial.println(ssid); //Imprime na serial o nome da Rede Wi-Fi

  WiFi.begin(ssid, password); //Inicialização da comunicação Wi-Fi
  

  //Verificação da conexão
  while (tempo <= 30) { //Enquanto estiver aguardando status da conexão
    delay(1000);
    Serial.print("."); //Imprime pontos
    tempo = tempo + 1;
    Serial.println(tempo);
  }
  //Passou 30 segundos vem para esta parte
  if (WiFi.status() != WL_CONNECTED) { //Se não conectou em 30 segundos cria o wifi configurador
    Serial.println("Wifi não conectado!"); 
    WiFiManager wm;
    wm.autoConnect("AutoConnectAp");
    Serial.println("Conectado...");
  }

  Serial.println("");
  Serial.println("WiFi Conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP()); //Função para exibir o IP da ESP
  Serial.println(WiFi.softAPmacAddress()); // Printa o mac adress

  server.on("/", handle_OnConnect); //Servidor recebe uma solicitação HTTP - chama a função handle_OnConnect
  server.onNotFound(handle_NotFound); //Servidor recebe uma solicitação HTTP não especificada - chama a função handle_NotFound

  server.begin(); //Inicializa o servidor
  Serial.println("Servidor HTTP inicializado");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  server.handleClient(); //Chama o método handleClient()
  handle_OnConnect();
  delay(1000);
  FirebasePush();
}

void handle_OnConnect() {
  temperatura = dht.readTemperature();  //Realiza a leitura da temperatura
  umidade = dht.readHumidity(); //Realiza a leitura da umidade

  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperatura); //Imprime no monitor serial o valor da temperatura lida
  Serial.print(" ºC  ");
  Serial.print("Umidade: ");
  Serial.print(umidade); //Imprime no monitor serial o valor da umidade lida
  Serial.println(" %");
  server.send(200, "text/html", EnvioHTML(temperatura, umidade)); //Envia as informações usando o código 200, especifica o conteúdo como "text/html" e chama a função EnvioHTML

}

void handle_NotFound() { //Função para lidar com o erro 404
  server.send(404, "text/plain", "Não encontrado"); //Envia o código 404, especifica o conteúdo como "text/pain" e envia a mensagem "Não encontrado"

}

String EnvioHTML(float Temperaturastat, float Umidadestat) { //Exibindo a página da web em HTML
  String ptr = "<!DOCTYPE html> <html>\n"; //Indica o envio do código HTML
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"; //Torna a página da Web responsiva em qualquer navegador Web
  ptr += "<meta http-equiv='refresh' content='2'>";//Atualizar browser a cada 2 segundos
  ptr += "<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\" rel=\"stylesheet\">\n";
  ptr += "<title>Monitor de Temperatura e Umidade</title>\n"; //Define o título da página

  //Configurações de fonte do título e do corpo do texto da página web
  ptr += "<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #000000;}\n";
  ptr += "body{margin-top: 50px;}\n";
  ptr += "h3 {margin: 50px auto 30px;}\n";
  ptr += "h4 {margin: 40px auto 20px;}\n";
  ptr += "p {font-size: 12px;color: #000000;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h4>Aviario 1</h4>\n";

  //Exibe as informações de temperatura e umidade na página web
  ptr += "<p><b>Temperatura: </b>";
  ptr += (float)Temperaturastat;
  ptr += " C</p>";
  ptr += "<p><b>Umidade: </b>";
  ptr += (float)Umidadestat;
  ptr += " %</p>";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;

}

void FirebasePush() {
  if (!isnan(umidade) || !isnan(temperatura)) {
    Firebase.setFloat( mac + "/DHT11/temperatura", temperatura);
    Firebase.setFloat( mac + "/DHT11/umidade", umidade);
    Serial.println("Publicado Firebase...");
  } else {
    Serial.println("Erro de publicação no Firebase");
  }
}
