#include <M5StickCPlus.h>
#include <cmath>

// Define the coordinates of points A, B, C, D, and the M5StickC
float pointA[3] = {0.0, 0.0, 0.0}; // front left bottom of the shelf
float pointB[3] = {1.0, 0.0, 0.0}; // back left bottom of the shelf
float pointC[3] = {1.0, 1.0, 0.0}; // back right bottom of the shelf
float pointD[3] = {0.0, 1.0, 0.0}; // front right bottom of the shelf (assuming this is what you intended for the top view)
float m5StickC[3] = {0.5, 0.5, 0.0}; // coordinate of M5StickC item, might have more 

void setup() {
  M5.begin();
  Serial.begin(115200);
}

void loop() {
  float* dimensions = calculateShelveDimensions(pointA, pointB, pointC, pointD);
  
  Serial.print("Length: "); // X-axis difference (point C to point B)
  Serial.print(dimensions[0]);
  Serial.print(" | Depth: "); // Y-axis difference (point D to point A)
  Serial.print(dimensions[1]);
  Serial.print(" | Height: "); // Z-axis (assuming height is vertical)
  Serial.println(dimensions[2]);

  bool isInside = isItemInsideShelf(m5StickC, pointA, pointB, pointC, pointD);
  if (isInside) {
    Serial.println("The item is inside the shelf.");
    findItem(m5StickC, dimensions);
  } else {
    Serial.println("The item is not inside the shelf.");
  }

  // Deallocate memory allocated for dimensions array
  delete[] dimensions;

  delay(1000); // delay for a second
}

float calculateDistance(float point1[3], float point2[3]) {
  return sqrt(pow(point2[0] - point1[0], 2) + pow(point2[1] - point1[1], 2) + pow(point2[2] - point1[2], 2));
}

float* calculateShelveDimensions(float pointA[3], float pointB[3], float pointC[3], float pointD[3]) {
  // Allocate memory for the dimensions array
  float* dimensions = new float[3];

  // Calculate width (the distance between point A and point B)
  dimensions[0] = calculateDistance(pointA, pointB);

  // Calculate height (the distance between point B and point D)
  dimensions[1] = calculateDistance(pointB, pointD);

  // Calculate length (the distance between point B and point C)
  dimensions[2] = calculateDistance(pointB, pointC);

  // Return the dimensions array
  return dimensions;
}

bool isItemInsideShelf(float m5StickC[3], float pointA[3], float pointB[3], float pointC[3], float pointD[3]) {
  // Check if the M5StickC's x coordinate is between pointB's x and pointC's x
  bool withinXRange = m5StickC[0] >= pointB[0] && m5StickC[0] <= pointC[0];
  
  // Check if the M5StickC's y coordinate is between pointD's y and pointB's y
  bool withinYRange = m5StickC[1] >= pointD[1] && m5StickC[1] <= pointB[1];
  
  // Check if the M5StickC's z coordinate is between pointA's z and pointB's z
  bool withinZRange = m5StickC[2] >= pointA[2] && m5StickC[2] <= pointE[2]; // Assuming point E is the top.

  return withinXRange && withinYRange && withinZRange; 
}



void findItem(float m5StickC[3], float dimensions[3]) {
  char level = 'A' + (int)(m5StickC[2] / (dimensions[2] / 3.0));
  int position = ((int)(m5StickC[1] / (dimensions[1] / 3.0)) * 3) + (int)(m5StickC[0] / (dimensions[0] / 3.0)) + 1;

  Serial.print("Location: ");
  Serial.print(level);
  Serial.println(position);
}

int compareCoordinate(float coordinate, float segmentSize) {
  if (coordinate <= segmentSize) {
    return 1;
  } else if (coordinate <= 2 * segmentSize) {
    return 2;
  } else {
    return 3;
  }
}


