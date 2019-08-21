// RESET THE ARDUINO TO RESET THE GAME!!!

/* Pin assignment...
  JAPAN       7
  INDIA       6
  MADAGASCAR  5
  SWITZERLAND 4
  NORWAY      3
  SOUTH_AM    2
  NORTH_AM    1
*/

#include <Servo.h>

#define is_Pressed(index) !digitalRead(index)    // Reading is LOW if button is pressed.

#define GROUND_PIN  0     // For the buttons. This pin is simply set to LOW the whole time.
#define LED_PIN     11    // Connect this pin to external LED.
#define SERVO_PIN   9     // Connect this pin to servo motor.

Servo servo;

// Location indexing. This indexing FOLLOWS the pin assignment.
enum loc_index
{
  NORTH_AM = 1,    // Index = 1
  SOUTH_AM,        // Index = 2, and so on.
  NORWAY,
  SWITZERLAND,
  MADAGASCAR,
  INDIA,
  JAPAN
};

// Sequence of locations. CHANGE THE SEQUENCE HERE.
byte sequence[7] =
{
  NORWAY,
  SWITZERLAND,
  JAPAN,
  INDIA,
  NORTH_AM,
  MADAGASCAR,
  SOUTH_AM
};


void setup() {

  //-------------- Setup --------------//
  // Set all input pins.
  for (byte i = 1; i <= 7; i++)
    pinMode(i, INPUT_PULLUP);

  pinMode(GROUND_PIN, OUTPUT);
  digitalWrite(GROUND_PIN, LOW);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  servo.attach(SERVO_PIN);
  servo.write(90);          // Write to initial position.
  delay(800);
  servo.detach();


  //-------------- Main Game --------------//

  while (1)    // Keep looping forever until the user input is correct.
  {
    byte user_input[7] = {0};
    byte input_no = 0;

    // Let the player input all 7 places first.
    while (input_no < 7)
    {
      // Scan across all the buttons...
      for (byte index = 1; index <= 7; index++)
      {
        if (is_Pressed(index))
        {
          user_input[input_no] = index;
          input_no++;

          digitalWrite(LED_PIN, HIGH);

          while (is_Pressed(index));   // Wait until the button is released.
          delay(200);                  // Debouncing.
          digitalWrite(LED_PIN, LOW);
          break;
        }
      }
    }

    // At this point, all 7 inputs should have been made.
    // Check if the user input is according to the sequence wanted.

    digitalWrite(LED_PIN, LOW);
    delay(500);

    byte correct_count = 0;
    for (byte i = 0; i < 7; i++)
    {
      if (user_input[i] == sequence[i])
        correct_count++;
    }

    if (correct_count == 7)     // If all the input are in the correct sequence...
    {
      for (byte i = 0; i < 2; i++)
      {
        digitalWrite(LED_PIN, HIGH);
        delay(150);
        digitalWrite(LED_PIN, LOW);
        delay(150);
      }
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
      
      servo.attach(SERVO_PIN);
      servo.write(0);          // Open the flap, release marble.
      delay(800);
      servo.detach();
      break;
    }
    else
    {
      for (byte i = 0; i < 3; i++)
      {
        digitalWrite(LED_PIN, HIGH);
        delay(80);
        digitalWrite(LED_PIN, LOW);
        delay(80);
      }
    }
  }
}

void loop() {}
