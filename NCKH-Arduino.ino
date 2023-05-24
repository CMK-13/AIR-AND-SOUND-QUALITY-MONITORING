
//                                          ĐỀ TÀI NGHIÊN CỨU KHOA HỌC NĂM 2 


//  Khai báo các thư viện :

    #include <SoftwareSerial.h>                                                             // Thư viện cổng kết nối trong phần mềm
    #include <DFRobotDFPlayerMini.h>                                                        // Thư viện DFPlayer Mini
    #include "Relay.h"                                                                      // Thư viện Relay

//  Thiết lập chân cảm biến và Arduino :

    const int A0_Sound        = A1;                                                         // Chân A0 của cảm biến nhận biết âm thanh
    const int A0_Gas          = A0;                                                         // Chân A0 của cảm biến khí 
    const int Read_Voltage    = A2;                                                         // Chân chuyển mạch nguồn
    const int TXDFP           = 2 ;                                                         // Nối chân TX của DFP vào chân số 2 của Arduino
    const int RXDFP           = 3 ;                                                         // Nối chân RX của DFP vào chân số 3 của Arduino
    const int TXESP8266       = 6 ;                                                         // Nối chân TX của ESP8266 vào chân số 6 của Arduino
    const int RXESP8266       = 7 ;                                                         // Nối chân RX của ESP8266 vào chân số 7 của Arduino
    const int Button_Gas      = 8 ;                                                         // Nút ấn xem tín hiệu chất lượng khí
    const int Button_Sound    = 9 ;                                                         // Nút ấn xem tín hiệu âm thanh
    const int Relay_Fan       = 10;                                                         // Kích xung cho chân relay mở quạt
    const int Relay_Source    = 11;                                                         // Kích xung cho chân chuyển nguồn

//  Khởi tạo đối tượng cho UART :

    SoftwareSerial myDFP ( TXDFP , RXDFP );                                                 // Khởi tạo cổng UART ảo
    SoftwareSerial myESP8266 ( TXESP8266 , RXESP8266 );                                     // Khởi tạo cổng UART ảo

//  Khởi tạo các đối tượng :

    DFRobotDFPlayerMini myDFPlayer;                                                         // Đối tượng cho DFPlayer    
    Relay Fan ( Relay_Fan , false );                                                        // Đối tượng cho Relay điều khiển quạt thông gió
    Relay Source ( Relay_Source , false );                                                  // Đối tượng cho Relay điều khiển nguồn

//  Thiết lập biến lưu trữ giá trị :

    int Gas_Sensor ;                                                                        // Đọc giá trị đầu ra trên chân analog của MQ135
    int Sound_Sensor ;                                                                      // Đọc giá trị đầu ra trên chân analog của KY-037
    int Gas_Max;                                                                            // Giá trị lớn nhất của cảm biến khí trong thời gian 3s đo
    int Sound_Max;                                                                          // Giá trị lớn nhất của cảm biến âm thanh trong thời gian 3s đo
    String flag = "Blue";                                                                   // Cờ trạng thái

//  Prototype của hàm :

    void Check_Connect_DFPLayer();                                                          // Kiểm tra kết nối với DFPlayer
    void Check_Connect_ESP8266();                                                           // Kiểm tra kết nối với ESP8266
    void Initialization_Pin ();                                                             // Thiết lập chân
    void Initialization_Baudrate ();                                                        // Thiết lập tốc độ đường truyền
    void Initialization_Variables();                                                        // Khởi tạo biến
    String Constraints_Status_Gas_Sensor ();                                                // Trạng thái ràng buộc của Gas
    String Constraints_Status_Sound_Sensor ();                                              // Trạng thái ràng buộc của âm thanh
    void Button_Gas_Status ();                                                              // Nút nhất Gas
    void Button_Sound_Status ();                                                            // Nút nhấn âm thanh
    void Alert_Status ();                                                                   // Hàm cảnh báo
    void Output_Gas_Sensor ();                                                              // Dữ liệu Gas
    void Output_Sound_Sensor ();                                                            // Dữ liệu Sound
    void Transmit_Data(int,int);                                                                   // Truyền dữ liệu sang ESP8266
    int Analysis_Gas_Data( int );                                                           // Phân tích dữ liệu khí 
    int Analysis_Sound_Data( int );                                                         // Phân tích dữ liệu âm thanh
    void Swap_Source ();                                                                    // Hàm chuyển đổi nguồn
    void Fan_Turn_Off();                                                                    // Hàm tắt quạt lọc

//  Hàm setup

    void setup()
    {
      Initialization_Pin ();
      Initialization_Baudrate ();
      Initialization_Variables ();
      Check_Connect_DFPLayer();                                 
      Check_Connect_ESP8266();                
      Gas_Sensor = Analysis_Gas_Data ( Gas_Sensor );                                         // Điều chỉnh thông số ô nhiễm không khí
      Sound_Sensor = Analysis_Sound_Data ( Sound_Sensor );                                   // Điều chỉnh thông số cường độ âm thanh
    }

//  Hàm loop

    void loop() 
    {
      /*
      delay(3000);
      Serial.print(analogRead(A0_Gas));
      Serial.print(",");
      Serial.println(analogRead(A0_Sound));
      */
      //Transmit_Data(Gas_Sensor,Sound_Sensor);                                            // Truyền dữ liệu đến ESP8266
      Alert_Status ();                                                                     // Trạng thái cảnh báo nếu có
      if ( Constraints_Status_Gas_Sensor () == "Blue" ) Button_Gas_Status ();              // Kiểm tra chất lượng không khí
      if ( Constraints_Status_Sound_Sensor () == "Blue" ) Button_Sound_Status ();          // Kiểm tra cường độ âm thanh
      else if ( Constraints_Status_Gas_Sensor () == "Yellow" )  Fan_Turn_Off();            // Tắt quạt lọc  
    }

//  Hàm phân tích dữ liệu khí

    int Analysis_Gas_Data ( int Gas_Sensor )
    {
      if ( Gas_Sensor > 200 ) return Gas_Sensor - 200;
      else if ( Gas_Sensor > 300 ) return Gas_Sensor - 200 ;
      else if ( Gas_Sensor > 400 ) return Gas_Sensor - 200;
      else return Gas_Sensor ; 
    }

//  Hàm phân tích dữ liệu âm thanh

    int Analysis_Sound_Data( int Sound_Sensor )
    {
      if ( Sound_Sensor > 300 )  return Sound_Sensor - 200 ;
      else if ( Sound_Sensor > 400 ) return Sound_Sensor - 200 ;
      else return Sound_Sensor - 200;
    }

//  Hàm khởi tạo biến

    void  Initialization_Variables()
    {
      Gas_Sensor = analogRead ( A0_Gas );
      Sound_Sensor = analogRead ( A0_Sound );
    }

//  Hàm kiểm tra kết nối đến DFPlayer

    void  Check_Connect_DFPLayer()
    {
        if ( myDFPlayer.begin ( myDFP ) ) 
          Serial.println(F("Connecting to DFPlayer Mini failed!"));                       // Lưu trữ trong bộ nhớ FLash ( F ( ) )
        else 
          Serial.println(F("Connecting to DFPlayer Mini succeed!"));  
    }

//  Hàm kiểm tra kết nối đến ESP8266

    void Check_Connect_ESP8266()
    {
      delay(500);
      if ( myESP8266.available() > 0 ) 
          Serial.println(F("Connecting to ESP8266 succeed!"));
      else    
          Serial.println(F("Connecting to ESP8266 failed!"));
    }

//  Hàm tắt quạt lọc

    void Fan_Turn_Off()
    {
      myDFPlayer.play(11);                                                        // Thông báo tắt quạt lọc
      Fan.turnOff();
    }

//  Hàm chuyển đổi nguồn

    void Swap_Source()
    {
      if ( analogRead ( Relay_Source ) < 990 ) Source.turnOff() ;                  // Nếu như điện áp nhỏ hơn 5V thì chuyển qua dùng nguồn pin.
    }

//  Hàm trạng thái ràng buộc của Gas

    String Constraints_Status_Gas_Sensor ()
    {
      if ( Gas_Sensor > 200  ) return "Red";                                      // Ô nhiễm không khí
      else if ( Gas_Sensor < 200 &&  flag == "Red" )                              // Vừa hết ô nhiễm không khí
      {
        flag = "Blue";
        return "Yellow";
      }
      else if ( Gas_Sensor  <  200  ) return "Blue";                              // Không ô nhiễm không khí
    }

//  Hàm trạng thái ràng buộc của âm thanh

    String Constraints_Status_Sound_Sensor ()
    {
      if ( Sound_Sensor > 200 ) return "Red";
      else return "Blue";
    }

//  Hàm khởi tạo chân pin

    void  Initialization_Pin ()
    {
      pinMode ( Button_Gas , INPUT_PULLUP );
      pinMode ( Button_Sound , INPUT_PULLUP );
      pinMode ( A0_Sound , INPUT );
      pinMode ( A0_Gas , INPUT );
    }

//  Hàm thiết lập tốc độ đường truyền

    void Initialization_Baudrate()
    {
      Fan.begin();
      myDFP.begin(9600);
      Serial.begin(115200);
      myESP8266.begin(115200);
      Source.begin();
    }

//  Hàm nút nhấn Gas

    void  Button_Gas_Status ()
    {
        if ( digitalRead ( Button_Gas ) == LOW )                                    // Phát hiện nhấn button
        { 
          unsigned long startMillis;                                                // biến để lưu thời điểm bắt đầu đo đạc
          unsigned int maxVal = 0;                                                  // biến để lưu giá trị lớn nhất đến thời điểm hiện tại
          myDFPlayer.volume(25);
          myDFPlayer.play(6);                                                       // Thông báo đo nồng độ không khí                 
          startMillis = millis();
          while (millis() - startMillis < 1000)                                     // Xác định khoảng thời gian 3s
          {
            if ( Gas_Sensor > Gas_Max ) Gas_Max = Gas_Sensor ;                      // Chọn giá trị lớn nhất trong số các giá trị mà cảm biến dọc được
          }
          Serial.print ("Giá trị cảm biến chất lượng không khí là : ");   
          Serial.println ( Gas_Sensor );
          delay(4000);
          Output_Gas_sensor ();
        }
    }

//  Hàm nút nhấn âm thanh

    void  Button_Sound_Status ()
    {
        if ( digitalRead ( Button_Sound ) == LOW )                                  // Phát hiện button
        {
          unsigned long startMillis;                                                // Biến để lưu thời điểm bắt đầu đo đạc
          unsigned int maxVal = 0;                                                  // Biến để lưu giá trị lớn nhất đến thời điểm hiện tại
          myDFPlayer.volume(25);
          myDFPlayer.play(5);                                                       // Thông báo đo cường độ âm thanh
          startMillis = millis();
          while (millis() - startMillis < 1000) 
          {
            if ( Sound_Sensor > Sound_Max ) Sound_Max = Sound_Sensor ;
          }
          Serial.print ("Giá trị cảm biến âm thanh là: ");
          Serial.println ( Sound_Sensor );
          delay(4000);
          Output_Sound_sensor (); 
        } 
        
    }

//  Hàm cảnh báo

    void  Alert_Status ()
    {
        
          if (  Gas_Sensor >=  200  && Sound_Sensor >= 200 )
          {
            myDFPlayer.volume(25);
            myDFPlayer.play(1);                                                     // Thông báo chất lượng không khí và âm thanh đều ở mức báo động
            delay(3000);
            myDFPlayer.play(2);                                                     // Phát âm thanh khởi động quạt lọc.
            Fan.turnOn();                                                           // Bật quạt lọc
            flag = "Red" ;       
            Serial.print ("Giá trị cảm biến chất lượng không khí là : ");   
            Serial.println ( Gas_Sensor ); 
            Serial.print ("Giá trị cảm biến âm thanh là: ");
            Serial.println ( Sound_Sensor );
            Transmit_Data(Gas_Sensor,Sound_Sensor);
          }                                                                   
      
        else if ( Sound_Sensor >= 200 ) 
        {
            myDFPlayer.volume(25);
            myDFPlayer.play(7);                                                      // Thông báo cường độ âm thanh ở mức báo động
            delay(5000);
            Serial.print ("Giá trị cảm biến âm thanh là: ");
            Serial.println ( Sound_Sensor );
            Transmit_Data(Gas_Sensor,Sound_Sensor);
        }
        else if ( Gas_Sensor >=  200 )
        {
            myDFPlayer.volume(25);       
            myDFPlayer.play(10);                                                     // Thông báo nồng độ không khí ở mức báo động
            delay(8000);
            myDFPlayer.play(2);                                                      // Phát âm thanh khởi động quạt lọc.
            Fan.turnOn();                                                            // Bật quạt lọc
            flag = "Red" ;       
            Serial.print ("Giá trị cảm biến chất lượng không khí là : ");   
            Serial.println ( Gas_Sensor );
            delay(5000);
    //        Gas_Sensor = 20;
            Transmit_Data(Gas_Sensor,Sound_Sensor);
        }
    }

//  Hàm dữ liệu khí gas

    void Output_Gas_sensor ()
    {
      myDFPlayer.volume(25);
      if ( Gas_Sensor < 100 ) myDFPlayer.play(3);                                   // Chất lượng không khí bình thường
      else if ( Gas_Sensor >= 100 && Gas_Sensor < 200 ) myDFPlayer.play(4);        // Chất lượng không khí trung bình
      Transmit_Data( Gas_Max , Sound_Max );
      delay(3000);
    }

//  Hàm dữ liệu âm thanh

    void Output_Sound_sensor ()
    {
      myDFPlayer.volume(25);
      if( Sound_Sensor < 100 ) myDFPlayer.play(9);                                  // Cường độ âm thanh bình thường
      else if( Sound_Sensor >= 100 && Sound_Sensor < 200 ) myDFPlayer.play(7);      // Cường độ âm thanh trung bình
      Transmit_Data( Gas_Max , Sound_Max );
      delay(3000);
    }

//  Hàm truyền dữ liệu sang ESP8266

    void Transmit_Data( int Sound_Sensor , int Gas_Sensor )
    {
      String Data = String ( Sound_Sensor ) + "," + String ( Gas_Sensor ) + "\n";
      myESP8266.print( Data );
    }

//

// Bài 1 :  Khu vực này đang ô nhiễm không khí và âm thanh mạnh cần tránh xa ngay lập tức
// Bài 2 :  Bật quạt lọc không khí
// Bài 3 :  Chất lượng không khí tốt không gây hại cho sức khoẻ
// Bài 4 :  Khu vực này có chất lượng không khí hơi tệ không nên tiếp xúc trong thời gian quá lâu
// Bài 5 :  Thông báo đo cường độ âm thanh chờ trong giây lát
// Bài 6 :  Thông báo đo chất lượng không khí chờ trong giây lát
// Bài 7 :  Khu vực này đang ô nhiễm tiếng ồn nghiêm trọng có thể gây hại cho sức khoẻ
// Bài 8 :  Khu vựa này đang rất ồn có thể gây đau tai cần tránh xa và có biện pháp cách âm
// Bài 9 :  Khu vực này không bị ô nhiễm tiếng ồn
// Bài 10 : Khu vực này khu vựa này đang ô nhiễm không khí rất nguy hiểm nếu tiếp xúc cần có đồ bảo hộ chuyển dụng
// Bài 11 : Hiện tại không khí đã hết nguy hiểm hệ thống đang tiến hành tắt quạt lọc








































