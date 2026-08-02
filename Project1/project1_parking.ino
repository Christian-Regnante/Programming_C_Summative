#include <stdlib.h>  // Required for C dynamic memory (malloc)
#include <stdbool.h> // Required for C boolean data types

// PIN DEFINITIONS
const int ENTRY_BUTTON_PIN = 12;
const int EXIT_BUTTON_PIN = 5;
const int TOTAL_SPACES = 4;

// LED Pins from left to right
const int LED_PINS[] = {11, 10, 9, 8};

// C-STYLE STRUCTURES & POINTERS
// In C, we use typedef to create an alias for the struct
typedef struct {
  int id;
  int ledPin;
  bool isOccupied; // false = available (LED ON), true = occupied (LED OFF)
} ParkingSpace;

// Pointer to an array of ParkingSpace pointers
ParkingSpace** parkingLot;

// NON-BLOCKING DEBOUNCE VARIABLES
unsigned long lastDebounceTimeEntry = 0;
unsigned long lastDebounceTimeExit = 0;
const unsigned long debounceDelay = 50;

int lastEntryButtonState = 1; // 1 is HIGH
int lastExitButtonState = 1;
int entryButtonState = 1;
int exitButtonState = 1;

// PARKING STATISTICS
int occupiedCount = 0;

// C FUNCTION PROTOTYPES
// Standard C practice requires declaring functions before they are called
void handleEntryButton(void);
void handleExitButton(void);
void processVehicleEntry(void);
void processVehicleExit(void);
void printStatistics(void);

void setup() {
  Serial.begin(9600);
  
  // Initialize buttons with internal pull-ups
  pinMode(ENTRY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EXIT_BUTTON_PIN, INPUT_PULLUP);
  
  // C-STYLE DYNAMIC MEMORY: Allocate memory for the array of pointers using malloc()
  parkingLot = (ParkingSpace**)malloc(TOTAL_SPACES * sizeof(ParkingSpace*));
  
  for (int i = 0; i < TOTAL_SPACES; i++) {
    // C-STYLE DYNAMIC MEMORY: Allocate memory for each specific structure using malloc()
    parkingLot[i] = (ParkingSpace*)malloc(sizeof(ParkingSpace));
    
    // Use pointer operator (->) to assign values
    parkingLot[i]->id = i + 1;
    parkingLot[i]->ledPin = LED_PINS[i];
    parkingLot[i]->isOccupied = false; 
    
    pinMode(parkingLot[i]->ledPin, OUTPUT);
    digitalWrite(parkingLot[i]->ledPin, HIGH); // Start with LEDs ON (Available)
  }
  
  Serial.println("=== Smart Parking System Initialized ===");
  printStatistics();
}

void loop() {
  handleEntryButton();
  handleExitButton();
}

// MODULAR FUNCTIONS

void handleEntryButton(void) {
  int reading = digitalRead(ENTRY_BUTTON_PIN);
  
  if (reading != lastEntryButtonState) {
    lastDebounceTimeEntry = millis(); // Reset non-blocking timer
  }
  
  if ((millis() - lastDebounceTimeEntry) > debounceDelay) {
    if (reading != entryButtonState) {
      entryButtonState = reading;
      
      if (entryButtonState == LOW) { 
        processVehicleEntry(); 
      }
    }
  }
  lastEntryButtonState = reading;
}

void handleExitButton(void) {
  int reading = digitalRead(EXIT_BUTTON_PIN);
  
  if (reading != lastExitButtonState) {
    lastDebounceTimeExit = millis(); // Reset non-blocking timer
  }
  
  if ((millis() - lastDebounceTimeExit) > debounceDelay) {
    if (reading != exitButtonState) {
      exitButtonState = reading;
      
      if (exitButtonState == LOW) { 
        processVehicleExit(); 
      }
    }
  }
  lastExitButtonState = reading;
}

void processVehicleEntry(void) {
  // ERROR HANDLING: Prevent exceeding maximum capacity
  if (occupiedCount >= TOTAL_SPACES) {
    Serial.println("[ERROR] Parking is FULL! Cannot admit more vehicles.");
    return;
  }
  
  // Find the first available space to park the car
  for (int i = 0; i < TOTAL_SPACES; i++) {
    if (parkingLot[i]->isOccupied == false) {
      parkingLot[i]->isOccupied = true;
      digitalWrite(parkingLot[i]->ledPin, LOW); // Turn OFF LED
      occupiedCount++;
      
      Serial.print("Action: Vehicle ENTERED. Slot ");
      Serial.print(parkingLot[i]->id);
      Serial.println(" is now OCCUPIED.");
      printStatistics();
      break; 
    }
  }
}

void processVehicleExit(void) {
  // ERROR HANDLING: Prevent negative capacity
  if (occupiedCount <= 0) {
    Serial.println("[ERROR] Parking is EMPTY! No vehicles to remove.");
    return;
  }
  
  // Find the first occupied space to free up
  for (int i = 0; i < TOTAL_SPACES; i++) {
    if (parkingLot[i]->isOccupied == true) {
      parkingLot[i]->isOccupied = false;
      digitalWrite(parkingLot[i]->ledPin, HIGH); // Turn ON LED
      occupiedCount--;
      
      Serial.print("Action: Vehicle EXITED. Slot ");
      Serial.print(parkingLot[i]->id);
      Serial.println(" is now AVAILABLE.");
      printStatistics();
      break; 
    }
  }
}

void printStatistics(void) {
  Serial.print("STATISTICS -> Occupied: ");
  Serial.print(occupiedCount);
  Serial.print(" | Available: ");
  Serial.println(TOTAL_SPACES - occupiedCount);
  Serial.println("----------------------------------------");
}