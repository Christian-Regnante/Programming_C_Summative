#ifndef PROJECT4_CONVERSIONS_H
#define PROJECT4_CONVERSIONS_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

/**
 * Function pointer type representing a unit conversion algorithm.
 * Accepts an input value (double) and returns converted value (double).
 */
typedef double (*ConversionFunc)(double input);

/**
 * Structure mapping a conversion name, unit labels, and its function pointer.
 */
typedef struct {
    const char *name;        // Descriptive name (e.g. "Celsius to Fahrenheit")
    const char *from_unit;   // Source unit label (e.g. "C")
    const char *to_unit;     // Target unit label (e.g. "F")
    ConversionFunc convert;  // Pointer to conversion function
} ConversionOption;

/* =========================================================================
 * CONVERSION MATH FUNCTIONS
 * ========================================================================= */
double celsius_to_fahrenheit(double c);
double fahrenheit_to_celsius(double f);
double km_to_miles(double km);
double miles_to_km(double mi);
double kg_to_pounds(double kg);
double pounds_to_kg(double lbs);
double cm_to_inches(double cm);
double inches_to_cm(double in);

/* =========================================================================
 * DISPATCH TABLE ACCESSORS
 * ========================================================================= */
const ConversionOption* get_conversion_option(int index);
int get_num_conversions(void);

#endif // PROJECT4_CONVERSIONS_H
