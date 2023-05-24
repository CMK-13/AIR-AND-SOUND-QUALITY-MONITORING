                                          // ĐỀ TÀI NGHIÊN CỨU KHOA HỌC

// Trước khi chạy file chúng ta cần phải cài đặt các thư viện bên dưới bằng cách add file zip, file này có thể tìm trên github


// Khai báo thư viện:

    #include <ESP8266WiFi.h>                                                 // Thư viện của ESP8266
    #include <WiFiClientSecure.h>                                            // Thư viện
    #include <SoftwareSerial.h>                                              // Thư viện khởi tạo cổng kết nối ảo
    #include <TinyGPS++.h>                                                   // Thư viện GPS

// Khởi tạo các biến lưu trữ thông tin

    const char* host = "script.google.com";                                  // Khởi tạo host
    const int httpsPort = 443;                                               // Khởi tạo công kết nối http
    const char* ssid = "Xiaomi";                                             // Tên wifi
    const char* password = "00000000";                                       // Mật khẩu wifi
    String Token = "AKfycbzeCOu0KsbojwLJQsTUyqodIWXx0tBi5chuZcwBlgGFzWJ9PfivL0ASPeUflkI_-DoMew";       // Token kết nối tới google sheet
    int Gas_Sensor;                                                                          // Biến lưu chất lượng không khí
    int Sound_Sensor;                                                                        // Biến lưu cường độ âm thanh
    const int TXGPS           = D3 ;                                                         // Nối chân TX của GPS vào chân D3 của ESP8266
    const int RXGPS           = D4 ;                                                         // Nối chân RX của GPS vào chân D4 của ESP8266
    const int TXARDUINO       = D5 ;                                                         // Nối chân TX của Arduino vào chân D5 của ESP8266 
    const int RXARDUINO       = D6 ;                                                         // Nối chân RX của Arduino vào chân D6 của ESP8266


// Khởi tạo cổng UART ảo:

    SoftwareSerial myGPS ( TXGPS , RXGPS );                                                  // Khởi tạo cổng UART ảo
    SoftwareSerial ArduinoSerial ( D5, D6 );                                                 // Khởi tạo đối tượng arduinoSerial với chân RX là D5 và chân TX là D6

// Khởi tạo đối tượng truy suất

    WiFiClientSecure client;                                                                // Đối tượng client 
    TinyGPSPlus mygps;                                                                      // Đối tượng cho GPS

// Các prototype của hàm:

    void Check_Connect_GPS();                                                                   // Kiểm tra kết nối với GPS
    void Send_Data_To_Google_Sheet( int , int );                                                 // Hàm truyền dữ liệu lên googlesheet
    void Get_Data_From_Arduino();                                                                // Hàm lấy dữ liệu từ Arduino
    void Get_Address_From_GPS();                                                                 // Hàm lấy dữ liệu từ GPS
    void Initialization_Baudrate();                                                              // Hàm khởi tạo tốc độ đường truyền kết nối
    void Check_Connect_To_Wifi();                                                                // Hàm kiểm tra kết nối tới Wifi
    void Send_Address_From_GPS_To_Google_Sheet();                                                // Hàm gửi dữ liệu từ GPS lên google sheet

// Hàm setup:

    void setup() 
    {
      Initialization_Baudrate();
      Check_Connect_GPS();
      Check_Connect_To_Wifi();
 //     Send_Address_From_GPS_To_Google_Sheet ();
    }

// Hàm loop:

    void loop() 
    {
      if ( ArduinoSerial.available() > 0 )  Get_Data_From_Arduino();                             // Kiểm tra xem có dữ liệu nhận được hay không, nếu có thì nhận dữ liệu 
    }

// Hàm thiết lập tốc độ đường truyền

    void Initialization_Baudrate()
    {
      Serial.begin(115200);
      ArduinoSerial.begin(115200);
      myGPS.begin(9600);
      WiFi.begin(ssid, password);
      Serial.print("Connecting");
    }

// Hàm lấy địa chỉ từ GPS:

    void Get_Address_From_GPS()
    {
      Serial.print(F("Location: ")); 
      if (mygps.location.isValid())
      {
        Serial.print(mygps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(mygps.location.lng(), 6);
      }
    }

// Hàm kiểm tra kết nối tới Wifi

    void Check_Connect_To_Wifi()
    {
      while (WiFi.status() != WL_CONNECTED)   Serial.print(".");
      Serial.println("");
      Serial.print("Successfully connected to : ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println();
      client.setInsecure();     // Bật chế độ kết nối không bảo mật
    }



// Hàm lấy dữ liệu từ Arduino:

    void Get_Data_From_Arduino()
    {
        String receivedData = ArduinoSerial.readStringUntil('\n'); // Nhận biết đánh dấu kết thúc chuỗi để phân biệt các chuỗi truyền liên tiếp
        int commaIndex = receivedData.indexOf(',');         // Tìm vị trí dấu phẩy trong chuỗi gửi đến
        if (commaIndex != -1) 
        {
        String Gas_Sensor_String = receivedData.substring ( 0, commaIndex );
        String Sound_Sensor_String = receivedData.substring ( commaIndex + 1 );
        Gas_Sensor =   Gas_Sensor_String.toInt();       // Giá trị của cảm biến không khí
        Sound_Sensor = Sound_Sensor_String.toInt();     // Giá trị của cảm biến âm  
        }
        delay(12000);
        Send_Data_To_Google_Sheet( Sound_Sensor, Gas_Sensor );
    }

// Hàm kiểm tra kết nối tới GPS:

    void  Check_Connect_GPS()
    {
      delay(500);
      if (mygps.charsProcessed() < 10)
        Serial.println(F("Connecting to GPS Module failed!"));
      else 
        Serial.println(F("Connecting to GPS Module Succed"));
        Get_Address_From_GPS();
    }

// Hàm gửi dữ liệu lên google sheet:

    void Send_Data_To_Google_Sheet(int Sound_SenSor, int Gas_Sensor )
    {
      Serial.print("connecting to ");
      Serial.println(host);

      if (!client.connect(host, httpsPort)) 
      {
        Serial.println("connection failed");
        return;
      }

      String url = "/macros/s/" + Token + "/exec?amthanh=" + String( Sound_Sensor ) + "&khongkhi=" + String( Gas_Sensor );
      Serial.println(url);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + host + "\r\n" +
                  "User-Agent: BuildFailureDetectorESP8266\r\n" +
                  "Connection: close\r\n\r\n");
      Serial.println("OK");
  }
// Hàm gửi địa chỉ của GPS lên google sheet:
/*
    void Send_Address_From_GPS_To_Google_Sheet ()
    {
      String latitude = String ( mygps.location.lat() );
      String longitude = String ( mygps.location.lng() ) ;
      String url = "/macros/s/" + Token + "/exec?kinhdo=" + longitude + "&vido=" + latitude ;
      Serial.println(url);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + host + "\r\n" +
                  "User-Agent: BuildFailureDetectorESP8266\r\n" +
                  "Connection: close\r\n\r\n");
      Serial.println("OK");
    } 
    */
















// END




