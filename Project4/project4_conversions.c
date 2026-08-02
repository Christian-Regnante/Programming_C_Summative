#include "project4_conversions.h"

/* =========================================================================
 * CONVERSION MATH IMPLEMENTATIONS
 * ========================================================================= */

double celsius_to_fahrenheit(double c) {
    return (c * 9.0 / 5.0) + 32.0;
}

double fahrenheit_to_celsius(double f) {
    return (f - 32.0) * 5.0 / 9.0;
}

double km_to_miles(double km) {
    return km * 0.621371;
}

double miles_to_km(double mi) {
    return mi * 1.60934;
}

double kg_to_pounds(double kg) {
    return kg * 2.20462;
}

double pounds_to_kg(double lbs) {
    return lbs * 0.453592;
}

double cm_to_inches(double cm) {
    return cm * 0.393701;
}

double inches_to_cm(double in) {
    return in * 2.54;
}

/* =========================================================================
 * CONVERSION DISPATCH TABLE
 * ========================================================================= */

static const ConversionOption CONVERSION_TABLE[] = {
    {"Celsius to Fahrenheit", "C",   "F",   celsius_to_fahrenheit},
    {"Fahrenheit to Celsius", "F",   "C",   fahrenheit_to_celsius},
    {"Kilometers to Miles",   "km",  "mi",  km_to_miles},
    {"Miles to Kilometers",   "mi",  "km",  miles_to_km},
    {"Kilograms to Pounds",   "kg",  "lbs", kg_to_pounds},
    {"Pounds to Kilograms",   "lbs", "kg",  pounds_to_kg},
    {"Centimeters to Inches", "cm",  "in",  cm_to_inches},
    {"Inches to Centimeters", "in",  "cm",  inches_to_cm}
};

static const int NUM_CONVERSIONS = sizeof(CONVERSION_TABLE) / sizeof(CONVERSION_TABLE[0]);

const ConversionOption* get_conversion_option(int index) {
    if (index < 0 || index >= NUM_CONVERSIONS) {
        return NULL;
    }
    return &CONVERSION_TABLE[index];
}

int get_num_conversions(void) {
    return NUM_CONVERSIONS;
}
