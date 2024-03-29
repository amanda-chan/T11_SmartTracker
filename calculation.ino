#include <M5StickCPlus.h>

// Define the coordinates of points A, B, C, D, and the M5StickC
float pointA[3] = {0.0, 0.0, 0.0}; // front left bottom of the shelve
float pointB[3] = {1.0, 0.0, 0.0}; // back left bottom of the shelve
float pointC[3] = {1.0, 1.0, 0.0}; // back right bottom of the shelve
float pointD[3] = {0.0, 1.0, 0.0}; // back left top of the shelve
float m5StickC[3] = {0.5, 0.5, 0.0}; // coordinate of m5 stick item, might have more 

void setup() {
  M5.begin();
  Serial.begin(115200);
}

void loop() {
  float* dimensions = calculateShelveDimensions(pointA, pointB, pointC, pointD);
  
  Serial.print("Width: ");
  Serial.print(dimensions[0]);
  Serial.print(" | Height: ");
  Serial.print(dimensions[1]);
  Serial.print(" | Length: ");
  Serial.println(dimensions[2]);
  
  // Deallocate memory allocated for dimensions array
  delete[] dimensions;

  findItem(m5StickC, dimensions);

  delay(1000); // delay
}

float* calculateShelveDimensions(float pointA[3], float pointB[3], float pointC[3], float pointD[3]) {
  // Allocate memory for the dimensions array
  float* dimensions = new float[3];

  // Calculate width
  dimensions[0] = pointB[2] - pointA[2]; // Z coordinate of point B - Z coordinate of point A

  // Calculate height
  dimensions[1] = pointD[1] - pointB[1]; // Y coordinate of point D - Y coordinate of point B

  // Calculate length
  dimensions[2] = pointC[0] - pointB[0]; // X coordinate of point C - X coordinate of point B

  // Return the dimensions array
  return dimensions;
}

float* calculateShelveDimensions() {
  // Allocate memory for the dimensions array
  float* dimensions = new float[3];

  // Calculate width
  dimensions[0] = pointB[2] - pointA[2]; // Z coordinate of point B - Z coordinate of point A

  // Calculate height
  dimensions[1] = pointD[1] - pointB[1]; // Y coordinate of point D - Y coordinate of point B

  // Calculate length
  dimensions[2] = pointC[0] - pointB[0]; // X coordinate of point C - X coordinate of point B

  // Return the dimensions array
  return dimensions;
}


// column --- length
// row    --- height
// depth  --- width 
// void findItem(float m5StickC[3], float dimensions[3]) {
//   int column = compareCoordinate(m5StickC[0], dimensions[2] / 3.0);
//   int row = compareCoordinate(m5StickC[1], dimensions[1] / 3.0);
//   int depth = compareCoordinate(m5StickC[2], dimensions[0]/3.0); 

//   Serial.print("Column: ");
//   Serial.print(column);
//   Serial.print(" | Row: ");
//   Serial.print(row);
//   Serial.print(" | Depth: ");
//   Serial.println(depth);
// }

void findItem(float m5StickC[3], float dimensions[3]) {
  char level = 'A' + (int)(m5StickC[2] / (dimensions[1] / 3.0));
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
